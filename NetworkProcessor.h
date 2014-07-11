#pragma once

#define WEBFRONT_IP "127.0.0.1"
#define FOUNTAIN_IP "127.0.0.1"
#define WEBFRONT_PORT 11911
#define FOUNTAIN_PORT 11111

//TODO: Move these......
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <iostream>


class NetworkProcessor {
    public:
        NetworkProcessor(int port);
        ~NetworkProcessor();
        NetworkProcessor & operator=(const NetworkProcessor&) = delete;
        NetworkProcessor(const NetworkProcessor&) = delete;
        virtual void checkAndHandlePackets(void (*handler)(char const*));
        virtual void sendData(char const*, unsigned int);
        virtual void sendDataToFountain(char const*, unsigned int);
    private:
        int sock;
        struct sockaddr_in siMe, siOther;
};
