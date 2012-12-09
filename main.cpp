/* 
 * File:   main.cpp
 * Author: Alex Kersten
 *
 * Created on November 15, 2012, 10:47 PM
 * 
 * 
 */

//Define this before compiling to target the Windows or Unix network stack.
#define WINDOWS false

//Official build number, useful to describe feature changes without having to
//reference code hashes.
#define BUILD   1

//The port that this server will listen on for incoming connections from the
//Webfront.
#define LISTEN_PORT 11211

#include <cstdlib>
#include <iostream>

#include "UDPStack.h"
#include "UnixUDPStack.h"

using namespace std;

UDPStack* webfrontStack;

int main(int argc, char** argv) {
    cout << "Enlight Fountain Backend\n";
    cout << "Build " << BUILD << "\n";
    cout << __DATE__ << "\n\n";

    cout << "Setting up default global state...\n";

    if (WINDOWS) {
        cout << "No Windows implementation yet, sorry!";
        return 1;
    } else {
        cout << "Initializing Unix UDP stack...\n";
        webfrontStack = new UnixUDPStack(LISTEN_PORT);

    }


    cout << "Attempt block on " << webfrontStack->getPort() << "...\n\n";
    webfrontStack->startListening();
    
    //If we make it here, we've returned from startListening and this must
    //be an error.
    cout << "Early return from startListening()!\n";

    delete webfrontStack;
    return 0;
}

