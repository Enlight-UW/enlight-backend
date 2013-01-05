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
#define DELAY 50


#include <cstdlib>
#include <iostream>

#include "UDPStack.h"


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

/**
 * Callback from our UDPStack. This takes the request string and:
 *  - Verifies the SMK
 *  - Switches to the correct routine based on the opcode
 *  - Handles the request
 *  - Sends back a response if appropriate
 * 
 * @param requestString The blob from the Webfront.
 */
void handleServiceRequest(char* requestString) {
    //TODO: Let's ignore the first 64 bytes (service key) for now.

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
    opcode.bytes[3] = requestString[64];
    opcode.bytes[2] = requestString[65];
    opcode.bytes[1] = requestString[66];
    opcode.bytes[0] = requestString[67];

    switch (opcode.value) {
        case 1:
            //Stop the server

            break;
        case 2:
            //Echo request

            break;
        case 3:
            //Update status request
            break;

    }


}

int main(int argc, char** argv) {
    cout << "Running sanity check...\n\n";

    //Check size of primative types so we can do some quick and dirty casting
    //in other parts of the code.
    if (sizeof (int) != 4 * sizeof (char)) {
        cout << "Primitive type size assumption was incorrect - our ghetto cast"
                << " won\'t work. Please run on a platform where ints are 32 "
                << "bits and chars are 8.";

        return sizeof (int) + sizeof (char);
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


    cout << "Enlight Fountain Backend\n";
    cout << "Build " << BUILD << "\n";
    cout << __DATE__ << "\n\n";

    cout << "Setting up default global state...\n";

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

    //If we make it here, it's done.
    cout << "Leaving main.\n";

    delete webfrontStack;
    return 0;
}

