/* 
 * File:   UnixUDPStack.cpp
 * Author: Alex Kersten
 * 
 * Created on December 8, 2012, 2:20 AM
 * 
 * The Unix implementation of our UDP stack; this may not compile under Windows
 * unless you're very careful about getting your Cygwin/other includes right.
 * 
 * Just delete this file and set WINDOWS to true in main.cpp if you encounter
 * difficulties compiling on Windows.
 * 
 * Credit:
 * See a basic Unix UDP implementation at http://www.abc.se/~m6695/udp.html
 * Needed a few modifications and extra casts to make it work in C++ (C isn't as
 * strict about pointer types)
 */

#include "UnixUDPStack.h"

#define BUFLEN 1024


using namespace std;

UnixUDPStack::~UnixUDPStack() {
    //Don't think we created any objects here!

    //Just close the socket.
    close(sock);
}

/**
 * This is called by the initialization list in the header file - it's basically
 * the constructor, because I didn't want all this nasty socket code to take up
 * space in the header (which already had an init list).
 */
void UnixUDPStack::pseudoconstruct() {
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        //Failed to create socket...
        cout << "[UnixUDPStack] Socket initialize failed\n";
        return;
    }

    memset((char*) &siMe, 0, sizeof (siMe));

    siMe.sin_family = AF_INET;
    siMe.sin_port = htons(this->getPort());
    siMe.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (sockaddr*) & siMe, sizeof (siMe)) == -1) {
        cout << "[UnixUDPStack] Binding failed, check for usage\n";
        return;
    }


    cout << "[UnixUDPStack] Entering non-blocking mode...\n";

    //Magic to make the socket not block when checking for packets...
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        cout << "[UnixUDPStack] Error: Can't prevent blocking!\n";
        exit(1);
    }
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    //End of magic

    cout << "[UnixUDPStack] Bound, ready to listen!\n\n";
}

/**
 * Called by our "scheduler" to check for packets. Needs to peek in order to not
 * block, as there are other important things to do if packets are not available
 * like continue sending information to the fountain.
 * 
 * I'm hesitant to use threads because C++11 isn't supported in the wonderful
 * version (3.4.4) of GCC that I'm using and I don't feel like upgrading, or
 * using any more libraries than I have to. Besides, it's easier this way.
 */
void UnixUDPStack::checkAndHandlePackets() {
    char buffer[BUFLEN];
    int bytesRead = 0;
    int siOtherLen = sizeof (siOther);

    //We're currently bound to the port. Receive incoming information! For now
    //just print it to standard out.

    //Clear the buffer before the next read.
    memset(&buffer, 0, BUFLEN);

    //No buffer overflow - recvfrom is smart enough to prevent that.
    if ((bytesRead = recvfrom(sock, buffer, BUFLEN, 0,
            (sockaddr*) & siOther, &siOtherLen)) == -1) {
        
        //No input yet, and because we're non-blocking, return right away to
        //allow for other processing to take place.
        return;
    }

    //Terminate if it was too long.
    memset(&(buffer[BUFLEN - 1]), '\0', 1);

    printf("[%s:%d] %s\n",
            inet_ntoa(siOther.sin_addr), ntohs(siOther.sin_port), buffer);
}
