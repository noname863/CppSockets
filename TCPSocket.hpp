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
    
    template <class... waitArgs>
    int connectImpl(const IpAddress & address, uint16_t port, waitArgs &&... args);
    
    void makeNonblocking();
    
    bool waitReadFromTimeval(timeval *);
    bool waitWriteFromTimeval(timeval *);
    
    template <class PtrType, long func(int, PtrType, size_t, int), void (TCPSocket::*waitMethod)()>
    int blockingOp(PtrType data, size_t size);
    
    template <class PtrType, long func(int, PtrType, size_t, int)>
    int nonblockingOp(PtrType data, size_t size, size_t & processed);
    
public:
    TCPSocket();
    TCPSocket(int socketHandle);
    int getSocketHandle();
    int getError();    
    int connect(const IpAddress &, uint16_t port);
    int connect(const IpAddress &, uint16_t port, std::chrono::microseconds &);    
    int close();
    bool isConnected();
    void setBlocking(bool);
    bool isBlocking();
    int setNagle(bool);
    bool isNagle();
    int flush();
    int send(const void * data, size_t size);
    int send(const void * data, size_t size, size_t & sent);
    int receive(void * data, size_t size);
    int receive(void * data, size_t size, size_t & received);
    bool isWriteReady();
    bool isReadReady();
    void waitWriteReady();
    void waitReadReady();
    bool waitWriteReady(std::chrono::microseconds &);
    bool waitReadReady(std::chrono::microseconds &);
    std::pair<IpAddress, uint16_t> getRemoteAddressPort();
    std::pair<IpAddress, uint16_t> getLocalAddressPort();
};

#endif // TCPSOCKET_HPP
