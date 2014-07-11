#include "NetworkProcessor.h"

//Buffer length used for both input and output buffers.
#define BUFLEN 1024

//cRio's receive buffer is 256 bytes long.
#define FOUNTAIN_BUFLEN 256

#include <cstring>
#include <cstdlib>


using namespace std;

NetworkProcessor::NetworkProcessor(int port) {
    this->port = port;
    
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        //Failed to create socket...
        cout << "[NetworkProcessor] Socket initialize failed\n";
        return;
    }

    memset((char*) &siMe, 0, sizeof (siMe));

    siMe.sin_family = AF_INET;
    siMe.sin_port = htons(this->getPort());
    siMe.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (sockaddr*) & siMe, sizeof (siMe)) == -1) {
        cout << "[NetworkProcessor] Binding failed, check for usage\n";
        return;
    }


    cout << "[NetworkProcessor] Entering non-blocking mode...\n";

    //Magic to make the socket not block when checking for packets...
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) {
        cout << "[NetworkProcessor] Error: Can't prevent blocking!\n";
        exit(99);
    }
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    //End of magic

    cout << "[NetworkProcessor] Bound, ready to listen!\n\n";
}

int NetworkProcessor::getPort() const {
    return port;
}

NetworkProcessor::~NetworkProcessor() {
    //Don't think we created any objects here!

    //Just close the socket.
    close(sock);
}

/**
 * Called by our "scheduler" to check for packets. Needs to peek in order to not
 * block, as there are other important things to do if packets are not available
 * like continue sending information to the fountain.
 */
void NetworkProcessor::checkAndHandlePackets(void (*handler)(char const*)) {
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

    handler(buffer);
}

/**
 * Transmits the payload data to the Webfront.
 * @param payload The binary data to be transmitted to the Webfront. Maximum
 * length determined by BUFLEN - 1024 by default.
 */
void NetworkProcessor::sendData(char const* payload, unsigned int payloadLength) {
    struct sockaddr_in si_other;
    int outSock, siOtherLength;

    siOtherLength = sizeof (si_other);
    char outBuffer[BUFLEN];

    if ((outSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        cout << "[NetworkProcessor] Couldn't write out, socket in use.\n";
        return;
    }

    memset((char*) &si_other, 0, siOtherLength);
    memset(outBuffer, 0, BUFLEN);

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(WEBFRONT_PORT);

    if (inet_aton(WEBFRONT_IP, &si_other.sin_addr) == 0) {
        cout << "[NetworkProcessor] Couldn't parse Webfront address.\n";
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

        cout << "[NetworkProcessor] toWebfront Transmission failure.\n";
        return;
    } else {
        //Success
        // cout << "[NetworkProcessor] Transmitted: " << outBuffer << "\n";
    }

    close(outSock);
}

void NetworkProcessor::sendDataToFountain(char const* payload, unsigned int payloadLength) {
    struct sockaddr_in si_other;
    int outSock, siOtherLength;

    siOtherLength = sizeof (si_other);
    char outBuffer[FOUNTAIN_BUFLEN];

    if ((outSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        cout << "[NetworkProcessor] Couldn't write out, socket in use.\n";
        return;
    }

    memset((char*) &si_other, 0, siOtherLength);
    memset(outBuffer, 0, FOUNTAIN_BUFLEN);

    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(FOUNTAIN_PORT);

    if (inet_aton(FOUNTAIN_IP, &si_other.sin_addr) == 0) {
        cout << "[NetworkProcessor] Couldn't parse fountain address.\n";
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
            (payloadLength > FOUNTAIN_BUFLEN ? FOUNTAIN_BUFLEN : payloadLength));

    if (sendto(outSock, outBuffer, (payloadLength > FOUNTAIN_BUFLEN ? FOUNTAIN_BUFLEN : payloadLength), 0, (sockaddr*) & si_other,
            siOtherLength) == -1) {

        cout << "[NetworkProcessor] toFountain Transmission failure.\n";
        return;
    } else {
        //Success
        // cout << "[NetworkProcessor] Transmitted: " << outBuffer << "\n";
    }

    close(outSock);
}
