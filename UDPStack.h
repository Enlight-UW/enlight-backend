/* 
 * File:   UDPStack.h
 * Author: Alex Kersten
 *
 * Created on December 8, 2012, 2:17 AM
 * 
 * ABC which will either be created as a Windows or Unix stack, based on a
 * compile time definition. By-the-book polymorphism.
 * 
 * It's only a stack in the sense that it's the network implementation of this
 * program specifically... We're not changing the actual operating system or
 * anything ;)
 */

#ifndef UDPSTACK_H
#define	UDPSTACK_H

#define WEBFRONT_IP "127.0.0.1"
#define WEBFRONT_PORT 11911

class UDPStack {
public:
    UDPStack(int port);
    virtual ~UDPStack() = 0;
    virtual void checkAndHandlePackets(void (*handler)(char const*)) = 0;
    virtual void sendData(char const*, unsigned int) = 0;
    int getPort() const;
private:
    int port;
};

#endif	/* UDPSTACK_H */

