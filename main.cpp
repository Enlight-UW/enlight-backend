/* 
 * File:   main.cpp
 * Author: Alex Kersten
 *
 * Created on November 15, 2012, 10:47 PM
 * 
 * Main file for enlight-backend! Sets up the UDP listener and begins the
 * listen/event cycle.
 */

//Official build number, useful to describe feature changes without having to
//reference code hashes.
#define BUILD   1

//The port that this server will listen on for incoming connections from the
//Webfront.
#define LISTEN_PORT 11211

//How long to wait (in milliseconds) between process ticks. I recommend 50ms or
//so, because our fountain jet interval is 250ms and we want to be able to be on
//top of things there.
#define DELAY 50

//Even if a clock edge hwits, no more than (ACTUATION_SEPARATION / CLOCK) valves
//will be moved to the nextState at once. With A_S and CLK of 200, this
//means up to 5 valves per second will toggle.
#define ACTUATION_SEPARATION 200

//How long to wait between state updates to the fountain.
#define CLOCK 200

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include "UDPStack.h"
#include "GlobalStateTracker.h"

//We'll need the usleep function from unistd.h
#include <unistd.h>

//Platform-specific network implementation
#include "UnixUDPStack.h"


using namespace std;

UDPStack* webfrontStack;
GlobalStateTracker* stateTracker;

char const* const SERVICE_MASTER_KEY =
        "AF1993ADFE944E38FE8CED6E490D1BB16C6A20F7F36237753A2EAF5BF2503536";

int const SMK_LENGTH = 64;
int const API_KEY_LENGTH = 64;

int actuationTimer = 0;
int clockTimer = 0;

/**
 * Take the current state from our global state tracker and send it over UDP to
 * the fountain. Do it only when you should (clock edge) - not handled in this
 * function, so be careful.
 */
void sendStateToFountain() {
    //Craft the payload to send to the fountain.
    int southValveState = stateTracker->getValveState();
    int verticalStates = southValveState & 4095;
    int horizontalStates = southValveState & 16773120;

    //Align horizontal states to the LSB, just like vertical states
    horizontalStates >>= 12;


    //The format of the packet is two 2-byte values, padded with don't cares
    //in front, horizontal first. However, the cRIO protocol switches xC and xR
    //on us, so do that.

    //Extract the correct values for xC, xR
    int hR = horizontalStates & 2048;
    int hC = horizontalStates & 1024;
    int vR = verticalStates & 2048;
    int vC = verticalStates & 1024;

    //Shift them to where they should be
    hR >>= 1;
    hC <<= 1;
    vR >>= 1;
    vC <<= 1;

    //Clear the existing states at those positions
    horizontalStates &= 1023;
    verticalStates &= 1023;

    //Recombine
    horizontalStates += hR;
    horizontalStates += hC;
    verticalStates += vR;
    verticalStates += vC;

    int combinedStates = horizontalStates << 16;
    combinedStates += verticalStates;

    //This is probably little-endian,so format it according to the order the
    //crio expects.
    char payload[5];
    payload[0] = 1; //opcode for south end valves
    payload[1] = (horizontalStates & 3840) >> 8;
    payload[2] = horizontalStates & 255;
    payload[3] = (verticalStates & 3840) >> 8;
    payload[4] = verticalStates & 255;

    //Send it away!

    webfrontStack->sendDataToFountain((char const*) (&payload),
            5);
}

bool stateIsDirty = false;

/**
 * Invoked by the main loop to handle global events. Happens after the reading
 * and processing of UDP packets, so things that would go here include code that
 * maintains or changes the current pattern being sent to the fountain, as well
 * as updating the global state and sending it to the Webfront. We'll do this
 * every delay period if it was requested (don't want stale states to pile up in
 * the hardware buffer) and PHP will see it a tenth of a second or so later.
 */
void globalProcess() {
    //Move us nicely to the next valve state.
    actuationTimer += DELAY;


    if (actuationTimer >= ACTUATION_SEPARATION) {
        actuationTimer = 0;
        if (stateTracker->ease()) {
            stateIsDirty = true;
        }
    }


    //Send the updated state to the fountain.

    clockTimer += DELAY;

    if (clockTimer >= CLOCK) {
        clockTimer = 0;
        
        //Save network traffic by not updating if nothing's changed.
        if (stateIsDirty) {
            sendStateToFountain();
            stateIsDirty = false;
        }
    }
}

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
            stateTracker->setNextValveState(atoi(requestString + 4 + SMK_LENGTH + API_KEY_LENGTH));

            break;
        case 6:
            //SetRestrictState

            //TODO: Priority check

            //Same assumption as before
            stateTracker->setRestrictState(atoi(requestString + 4 + SMK_LENGTH + API_KEY_LENGTH));
            break;
        case 7:
            //ToggleValveState
            //TODO: Check priorities, etc.
            //Assume there's no parameter after this one - if there is, atoi
            //won't see the \0 and bad things will happen.
            stateTracker->setNextValveState(stateTracker->getNextValveState() ^
                    atoi(requestString + 4 + SMK_LENGTH + API_KEY_LENGTH));

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


    cout << "Enlight Fountain Backend\n";
    cout << "Build " << BUILD << "\n";
    cout << __DATE__ << "\n\n";

    cout << "Setting up default global state...\n";
    stateTracker = new GlobalStateTracker();

    cout << "Initializing UDP stack...\n";
    webfrontStack = new UnixUDPStack(LISTEN_PORT);

    cout << "Entering main loop, listening on "
            << webfrontStack->getPort() << "...\n\n";

    cout << "TODO: Change the cRIO port and address to the correct values. We "
            << "don't want them in the commit log, and we're probably going to "
            << "wind up storing them in a flatfile somewhere." << endl;

    //TODO: Add sever shutdown capability based on received packet
    for (;;) {
        //Forever, check for a single packet from the Webfront, process it,
        webfrontStack->checkAndHandlePackets(&handleServiceRequest);

        //Do global processing.
        globalProcess();

        //Sleep for the right amount of time - usleep is microseconds.
        usleep(1000 * DELAY);
    }


    delete webfrontStack;
    delete stateTracker;
    return 0;
}

