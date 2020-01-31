#ifndef TCPSOCKET_HPP
#define TCPSOCKET_HPP
#include "IpAddress.hpp"
#include <netinet/in.h>
#include <chrono>

class TCPSocket
{
    int socket;
    bool blocking;
    bool nagleEnabled;
    bool connected = false;
    bool closed;
    
    sockaddr_in createAddress(uint32_t address, uint16_t port);
    void create();
    void recreate();
    int getError();
public:
    TCPSocket();
    int getSocketHandle();
    int connect(const IpAddress &, uint16_t port);
    int connect(const IpAddress &, uint16_t port, std::chrono::microseconds &);    
    int close();
    bool isConnected();
    int setBlocking(bool);
    bool isBlocking();
    int setNagle(bool);
    bool isNagle();
    int flush();
    int send(const void * data, size_t size);
    int send(const void * data, size_t size, size_t & sent);
    int receive(const void * data, size_t size);
    int receive(const void * data, size_t size, size_t & received);
    bool isWriteReady();
    bool isReadReady();
    int waitWriteReady();
    int waitReadReady();
    int waitWriteReady(std::chrono::microseconds &);
    int waitReadReady(std::chrono::microseconds &);
    IpAddress getRemoteAddress();
    IpAddress getRemotePort();
    uint16_t getLocalPort();
};

#endif // TCPSOCKET_HPP
