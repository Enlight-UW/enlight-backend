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

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>


#define BUFLEN 1024

using namespace std;

UnixUDPStack::~UnixUDPStack() {
}

void UnixUDPStack::startListening() {
    char buffer[BUFLEN];
    struct sockaddr_in siMe, siOther;
    int siOtherLen = sizeof (siOther);

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

    cout << "[UnixUDPStack] Bound, listening!\n";

    //We're currently bound to the port. Receive incoming information! For now
    //just print it to standard out.
    for (;;) {
        if (recvfrom(sock, buffer, BUFLEN, 0, (sockaddr*) &siOther, &siOtherLen) == -1) {
            cout << "[UnixUDPStack] Receive failure\n";
            return;
        }

        printf("[%s:%d] %s\n", inet_ntoa(siOther.sin_addr), ntohs(siOther.sin_port), buffer);
    }

    close(sock);
}
