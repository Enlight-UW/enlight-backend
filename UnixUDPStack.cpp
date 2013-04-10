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

//Buffer length used for both input and output buffers.
#define BUFLEN 1024

#include <cstring>
#include <cstdlib>

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
        exit(99);
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
void UnixUDPStack::checkAndHandlePackets(void (*handler)(char const*)) {
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

    //Note - if you have any null bytes in your opcode (like most opcodes we've
    //got) then this will likely stop printing out the buffer after the SMK,
    //which is fairly useless (it'll only print the SMK, because it hits a \0).
    //Keeping it around though because it's a good reference for getting the
    //remote address.
    //printf("[%s:%d] %s\n",
    //inet_ntoa(siOther.sin_addr), ntohs(siOther.sin_port), buffer);

    //Handle this input - yes, buffer's got function scope but technically this
    //is still inside the function, so we don't need to worry about undefined
    //behavior.
    handler(buffer);
}

/**
 * Transmits the payload data to the Webfront.
 * @param payload The binary data to be transmitted to the Webfront. Maximum
 * length determined by BUFLEN - 1024 by default.
 */
void UnixUDPStack::sendData(char const* payload, unsigned int payloadLength) {
    struct sockaddr_in si_other;
    int outSock, siOtherLength;

    siOtherLength = sizeof (si_other);
    char outBuffer[BUFLEN];

    if ((outSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        cout << "[UnixUDPStack] Couldn't write out, socket in use.\n";
        return;
    }

    memset((char*) &si_other, 0, siOtherLength);
    memset(outBuffer, 0, BUFLEN);

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(WEBFRONT_PORT);

    if (inet_aton(WEBFRONT_IP, &si_other.sin_addr) == 0) {
        cout << "[UnixUDPStack] Couldn't parse Webfront address.\n";
        return;
    }

    //This might look dangerous, but there's a few cases here. First, know that
    //payloadLength is the size of the payload string sans the terminating \0.
    //This means that we'll need to append one if it's shorter than the buffer
    //size. Luckily, we fill this buffer earlier with \0's so it's okay. In the
    //case that we truncate the payload (because payloadLength > BUFLEN), there
    //will be no NULL byte at the end of the payload - but that's also okay
    //because our receiving end experts a payload of up to BUFLEN only, and will
    //only read that much.
    memcpy(outBuffer, payload,
            (payloadLength > BUFLEN ? BUFLEN : payloadLength));

    if (sendto(outSock, outBuffer, BUFLEN, 0, (sockaddr*) & si_other,
            siOtherLength) == -1) {

        cout << "[UnixUDPStack] Transmission failure.\n";
        return;
    } else {
        //Debug
       // cout << "[UnixUDPStack] Transmitted: " << outBuffer << "\n";
    }

    close(outSock);
}
