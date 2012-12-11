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
 * every delay period - doesn't matter, it's UDP - and PHP will surely see it
 * when the user invokes a request that requires the state to be updated by
 * the client.
 */
void globalProcess() {

}

int main(int argc, char** argv) {
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
        webfrontStack->checkAndHandlePackets();

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

