/* 
 * File:   UnixUDPStack.h
 * Author: Alex Kersten
 *
 * Created on December 8, 2012, 2:20 AM
 */

#ifndef UNIXUDPSTACK_H
#define	UNIXUDPSTACK_H

#include "UDPStack.h"

class UnixUDPStack : public UDPStack {
public:
    UnixUDPStack(int port) : UDPStack(port){};
    virtual ~UnixUDPStack();
    virtual void startListening();
private:

};

#endif	/* UNIXUDPSTACK_H */

