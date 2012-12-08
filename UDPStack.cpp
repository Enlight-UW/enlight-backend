/* 
 * File:   UDPStack.cpp
 * Author: Alex Kersten
 * 
 * Created on December 8, 2012, 2:17 AM
 */

#include "UDPStack.h"

UDPStack::UDPStack(int port) {
    this->port = port;
}

int UDPStack::getPort() const {
    return port;
}

UDPStack::~UDPStack() {
}

