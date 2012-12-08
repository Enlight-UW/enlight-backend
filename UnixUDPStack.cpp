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
 */


#include "UnixUDPStack.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>


UnixUDPStack::~UnixUDPStack() {
}


void UnixUDPStack::startListening() {
    
}
