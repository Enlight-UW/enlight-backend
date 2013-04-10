/* 
 * File:   main.cpp
 * Author: Alex Kersten
 *
 * Created on November 15, 2012, 10:47 PM
 * 
 * Main file for enlight-backend! Sets up the UDP listener and begins the
 * listen/event cycle.
 */

//Define this before compiling to target the Windows or Unix network stack.
#define WINDOWS false

//Official build number, useful to describe feature changes without having to
//reference code hashes.
#define BUILD   1

//The port that this server will listen on for incoming connections from the
//Webfront.
#define LISTEN_PORT 11211

//How long to wait (in milliseconds) between process ticks. I recommend 50ms or
//so, because our fountain jet interval is 250ms and we want to be able to be on
//top of things there.
#define DELAY 25


#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include "UDPStack.h"
#include "GlobalStateTracker.h"


//Windows vs Unix headers
#if WINDOWS

//We'll need the Sleep function from windows.h
#include <windows.h>
#else

//We'll need the usleep function from unistd.h
#include <unistd.h>

//Platform-specific network implementation
#include "UnixUDPStack.h"
#endif

using namespace std;

UDPStack* webfrontStack;
GlobalStateTracker* stateTracker;

char const* const SERVICE_MASTER_KEY =
        "AF1993ADFE944E38FE8CED6E490D1BB16C6A20F7F36237753A2EAF5BF2503536";

int const SMK_LENGTH = 64;
int const API_KEY_LENGTH = 64;

/**
 * Invoked by the main loop to handle global events. Happens after the reading
 * and processing of UDP packets, so things that would go here include code that
 * maintains or changes the current pattern being sent to the fountain, as well
 * as updating the global state and sending it to the Webfront. We'll do this
 * every delay period if it was requested (don't want stale states to pile up in
 * the hardware buffer) and PHP will see it a tenth of a second or so later.
 */
void globalProcess() {

}

int test = 0;

/**
 * Callback from our UDPStack. This takes the request string and:
 *  - Verifies the SMK
 *  - Switches to the correct routine based on the opcode
 *  - Handles the request
 *  - Sends back a response if appropriate
 * 
 * @param requestString The blob from the Webfront.
 */
void handleServiceRequest(char const* requestString) {
    //Verify SMK
    //The default SMK is:
    //     AF1993ADFE944E38FE8CED6E490D1BB16C6A20F7F36237753A2EAF5BF2503536     

    if (memcmp(SERVICE_MASTER_KEY, requestString, SMK_LENGTH) != 0) {
        //TODO: More info about _which_ client would potentially be useful here.
        cout << "SMK validation failed(!), will not honor request.\n";
        return;
    }

    //Parse out the opcode, at bytes 64-68 in the char array... This is under
    //the assumption that the bytes arriving in requestString are big-endian and
    //we're running on a little-endian platform, as well as assuming that chars
    //are 8 bits wide and integers are 32-bits. These assumptions are both
    //validated in the first few lines of main() so we should be fine doing
    //it this way, although there are nicer ways.

    union opcodeDecoder {
        int value;
        char bytes[4];
    } opcode;

    //Little endian platform, big endian encoding - switch the ordering around
    opcode.bytes[3] = requestString[SMK_LENGTH];
    opcode.bytes[2] = requestString[SMK_LENGTH + 1];
    opcode.bytes[1] = requestString[SMK_LENGTH + 2];
    opcode.bytes[0] = requestString[SMK_LENGTH + 3];

    //TODO: For certain cases, check to see if this user actually has control
    //at this point in time. For those cases, the first parameter of length
    //API_KEY_LENGTH will be their respective key.


    switch (opcode.value) {
        case 1:
            //Stop the server

            break;
        case 2:
            //Echo request

            //Walk the request string to find the null byte (which we're
            //guaranteed from our receive implementation).
            int echoLength;

            echoLength = -1;
            while (requestString[68 + (++echoLength)] != '\0') {
            }

            webfrontStack->sendData(&(requestString[68]), echoLength);
            cout << "[Echo] Length: " << echoLength << "\n";
            break;
        case 3:
            //Update status request
            stateTracker->generateSerializedState();

            webfrontStack->sendData(stateTracker->getSerializedStateValue(),
                    stateTracker->getSerializedStateSize());
            break;
        case 4:
            //STDEcho request - send the payload to standard out.

            //Don't worry, there's a null terminator on the end of this - check
            //UnixUDPStack.cpp if you're unsure.
            cout << "[STDEcho] " << (requestString + 68) << "\n";
            break;
        case 5:
            //SetValveState

            //TODO: Check priority of this request and see if the current API
            //key being used has a higher priority or not.

            //The new state will be past the SMK, API key and opcode

            //Assume there's no parameter after this one - if there is, atoi
            //won't see the \0 and bad things will happen.
            stateTracker->setValveState(atoi(requestString + 4 + SMK_LENGTH + API_KEY_LENGTH));

            break;
        case 6:
            //SetRestrictState

            //TODO: Priority check

            //Same assumption as before
            stateTracker->setRestrictState(atoi(requestString + 4 + SMK_LENGTH + API_KEY_LENGTH));
            break;

            //TODO: RequestControl opcode
    }
}

int main(int argc, char** argv) {
    //First is the sanity check to make sure the environment we're operating in
    //follows the assumptions that we make.

    cout << "Running sanity check...\n\n";

    //Check size of primitive types so we can do some quick and dirty casting
    //in other parts of the code.
    if (sizeof (int) != 4 * sizeof (char)) {
        cout << "Primitive type size assumption was incorrect - our ghetto cast"
                << " won\'t work. Please run on a platform where ints are 32 "
                << "bits and chars are 8.";

        return sizeof (int) + sizeof (char);
    }

    //Check that we've changed the default SMK; warn if we haven't.
    const char* defaultSMK
            = "AF1993ADFE944E38FE8CED6E490D1BB16C6A20F7F36237753A2EAF5BF2503536";
    if (memcmp(SERVICE_MASTER_KEY, defaultSMK, 64) == 0) {
        cout << "WARNING: THE SERVICE MASTER KEY IS STILL SET TO ITS DEFAULT "
                << "VALUE. ANYONE WHO VIEWS THE SOURCE ON GITHUB AND KNOWS THE "
                << "SERVER ADDRESS WILL BE ABLE TO TAKE CONTROL OF THE FOUNTAIN"
                << ". CHANGE THE VALUE IN main.cpp NAMED SERVICE_MASTER_KEY!\n";
    }

    //Check to see if this is a little endian platform...

    union sizeCheck {
        int readOut;
        char bytes[4];
    } check;

    check.bytes[0] = 1;
    check.bytes[1] = 0;
    check.bytes[2] = 0;
    check.bytes[3] = 0;

    if (check.readOut != 1) {
        cout << "This system is not little-endian. Not sure where you found it "
                << "but please put it out of its misery and assimilate to the "
                << "Intel master race... We've got cookies...\n";

        //But really, little endian is better.
        return 16777216;
    }



    //
    // End of sanity check
    //


    cout << "Enlight Fountain Backend\n";
    cout << "Build " << BUILD << "\n";
    cout << __DATE__ << "\n\n";

    cout << "Setting up default global state...\n";
    stateTracker = new GlobalStateTracker();

#if WINDOWS
    cout << "\nNo Windows implementation yet, sorry!\n";
    cout << "If someone wants to make one, extend UDPStack and implement "
            << "any virtual methods you see. Also, you may want to look at "
            << "how I've done it in UnixUDPStack in order to get the "
            << "socket listening in the constructor (use an initialization "
            << "list to call UDPStack:: with your port number). Then just "
            << "make sure you implement it such that you can peek at "
            << "incoming packets without actually blocking if there's "
            << "nothing there (important!) so that our no-thread model "
            << "keeps working. Basically, it should be transparent as to "
            << "whether the Unix or Windows stack is in use. I don't know "
            << "why you'd want to write a separate Windows stack though, "
            << "since the Unix one works just fine under Cygwin.";
    return 1;
#else
    cout << "Initializing Unix UDP stack...\n";
    webfrontStack = new UnixUDPStack(LISTEN_PORT);

#endif


    cout << "Entering main loop, listening on "
            << webfrontStack->getPort() << "...\n\n";


    //TODO: Add sever shutdown capability based on received packet
    for (;;) {
        //Forever, check for a single packet from the Webfront, process it,
        webfrontStack->checkAndHandlePackets(&handleServiceRequest);

        //Do global processing.
        globalProcess();

        //Sleep for the right amount of time based on the platform.
#if WINDOWS
        Sleep(DELAY);
#else
        //usleep(microseconds)
        usleep(1000 * DELAY);
#endif
    }


    delete webfrontStack;
    delete stateTracker;
    return 0;
}

