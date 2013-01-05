/* 
 * File:   UnixUDPStack.h
 * Author: Alex Kersten
 *
 * Created on December 8, 2012, 2:20 AM
 */

#ifndef UNIXUDPSTACK_H
#define	UNIXUDPSTACK_H

#include "UDPStack.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <iostream>

class UnixUDPStack : public UDPStack {
public:

    UnixUDPStack(int port) : UDPStack(port) {
        pseudoconstruct();
    };
    virtual ~UnixUDPStack();
    virtual void checkAndHandlePackets(void (*handler)(char*));
private:
    int sock;
    struct sockaddr_in siMe, siOther;
    void pseudoconstruct();
};

#endif	/* UNIXUDPSTACK_H */

