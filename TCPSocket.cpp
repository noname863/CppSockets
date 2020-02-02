#include "TCPSocket.hpp"
// TODO: add windows headers
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/select.h>

sockaddr_in TCPSocket::createAddress(uint32_t address, uint16_t port) {
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = address;
    addr.sin_family = AF_INET;
    addr.sin_port = port;

    return addr;
}

void TCPSocket::create() {
    nagleEnabled = true;
    blocking = true;
    closed = true;
    recreate();
}

void TCPSocket::recreate() {
    if (this->closed == true) {
        this->socket = ::socket(AF_INET, SOCK_STREAM, 0);
        this->closed = false;
        makeNonblocking();
        this->setNagle(nagleEnabled);
    }
}

void TCPSocket::makeNonblocking() {
    int flags = ::fcntl(this->socket, F_GETFL);
    
    // handle errors
    ::fcntl(this->socket, F_SETFL, flags | O_NONBLOCK);
}

bool TCPSocket::waitReadFromTimeval(timeval * t) {
    fd_set sockets;
    FD_ZERO(&sockets);
    FD_SET(this->socket, &sockets);
    bool result = ::select(this->socket + 1, &sockets, nullptr, nullptr, t);
    return result;
}

bool TCPSocket::waitWriteFromTimeval(timeval * t) {
    fd_set sockets;
    FD_ZERO(&sockets);    
    FD_SET(this->socket, &sockets);
    fd_set testErr;
    bool result = ::select(this->socket + 1, nullptr, &sockets, nullptr, t);
    return result;
}

int TCPSocket::getError() {
    return errno;
}

TCPSocket::TCPSocket() {
    create();
}

TCPSocket::TCPSocket(int socketHandle) {
    this->socket = socketHandle;
    this->closed = false;
    makeNonblocking();
    this->setNagle(true);
}

int TCPSocket::getSocketHandle() {
    return this->socket;
}

template <class... waitArgs>
int TCPSocket::connectImpl(const IpAddress & address, uint16_t port, waitArgs &&... args) {
    // create socket if it was closed
    recreate();
    
    sockaddr_in sockApiAddr = createAddress(address, htons(port));
    
    if (::connect(this->socket, reinterpret_cast<sockaddr *>(&sockApiAddr), sizeof(sockaddr_in)) == -1) {
        if (!this->blocking || getError() != EINPROGRESS) {
            return getError();            
        } else {
            waitWriteReady(std::forward<waitArgs>(args)...);
            int resultCode;
            socklen_t resultSize = sizeof(int);
            getsockopt(this->socket, SOL_SOCKET, SO_ERROR, &resultCode, &resultSize);
            return resultCode;
        }
    }
    return 0;
}

int TCPSocket::connect(const IpAddress & address, uint16_t port)
{
    return connectImpl(address, port);
}

bool TCPSocket::isWriteReady() {
    timeval t {0, 0};
    return waitWriteFromTimeval(&t);
}

bool TCPSocket::isReadReady() {
    timeval t = {0, 0};
    return waitReadFromTimeval(&t);
}

void TCPSocket::waitWriteReady() {
    waitWriteFromTimeval(nullptr);
}

void TCPSocket::waitReadReady() {
    waitReadFromTimeval(nullptr);
}

bool TCPSocket::waitWriteReady(std::chrono::microseconds & mc) {
    timeval t;
    t.tv_sec = mc.count() / 1000000;
    t.tv_usec = mc.count() % 1000000;
    return waitWriteFromTimeval(&t);
}

bool TCPSocket::waitReadReady(std::chrono::microseconds & mc) {
    timeval t;
    t.tv_sec = mc.count() / 1000000;
    t.tv_usec = mc.count() % 1000000;
    return waitReadFromTimeval(&t);
}

int TCPSocket::connect(const IpAddress & address, uint16_t port, std::chrono::microseconds & mc) {
    return connectImpl(address, port, mc);
}

int TCPSocket::close() {
    this->closed = true;
    return ::close(this->socket);
}

bool TCPSocket::isConnected() {
    return connected;
}

void TCPSocket::setBlocking(bool block) {
    this->blocking = block;
}

bool TCPSocket::isBlocking() {
    return this->blocking;
}

int TCPSocket::setNagle(bool nagle) {
    this->nagleEnabled = nagle;
    int flag = !nagle;
    if (::setsockopt(this->socket, IPPROTO_TCP, TCP_NODELAY, &flag, 4) == -1) {
        return getError();
    }
    return 0;
}

bool TCPSocket::isNagle() {
    return this->nagleEnabled;
}

int TCPSocket::flush() {
    if (this->nagleEnabled) {
        int result = 0;
        result = setNagle(false);
        if (result != 0) return result;
        void * emptyData = std::malloc(0);
        bool prevBlocking = this->blocking;
        this->blocking = true;
        result = send(emptyData, 0);
        if (result != 0) return result;
        this->blocking = prevBlocking;
        result = setNagle(true);
        return result;
    }
    return 0;
}

int TCPSocket::send(const void * data, size_t size) {
    if (!this->blocking) {
        std::cerr << "partial send of nonblocking socket is not handled!" << std::endl;
        size_t fictiveProcessed;
        return nonblockingOp<const void *, ::send>(data, size, fictiveProcessed);
    }
    return blockingOp<const void *, ::send, &TCPSocket::waitWriteReady>(data, size);
}

int TCPSocket::send(const void * data, size_t size, size_t & sent) {
    if (!this->blocking) {
        return nonblockingOp<const void *, ::send>(data, size, sent);
    }
    sent = size;
    return blockingOp<const void *, ::send, &TCPSocket::waitWriteReady>(data, size);
}

int TCPSocket::receive(void * data, size_t size) {
    if (!this->blocking) {
        std::cerr << "partial receive of nonblocking socket is not handled!" << std::endl;
        size_t fictiveProcessed;
        return nonblockingOp<void *, ::recv>(data, size, fictiveProcessed);
    }
    return blockingOp<void *, ::recv, &TCPSocket::waitReadReady>(data, size);
}

int TCPSocket::receive(void * data, size_t size, size_t & received) {
    if (!this->blocking) {
        return nonblockingOp<void *, ::recv>(data, size, received);
    }
    received = size;
    return blockingOp<void *, ::recv, &TCPSocket::waitReadReady>(data, size);
}

std::pair<IpAddress, uint16_t> TCPSocket::getRemoteAddressPort() {
    
    if (!this->closed)
    {
        // Retrieve informations about the local end of the socket
        sockaddr_in address;
        socklen_t size = sizeof(address);
        if (::getpeername(this->socket, reinterpret_cast<sockaddr*>(&address), &size) != -1)
        {
            return {address.sin_addr.s_addr, address.sin_port};
        }
    }

    // We failed to retrieve the port
    return {IpAddress(), 0};
}

std::pair<IpAddress, uint16_t> TCPSocket::getLocalAddressPort() {
    
    if (!this->closed)
    {
        // Retrieve informations about the local end of the socket
        sockaddr_in address;
        socklen_t size = sizeof(address);
        if (::getsockname(this->socket, reinterpret_cast<sockaddr*>(&address), &size) != -1)
        {
            return {address.sin_addr.s_addr, address.sin_port};
        }
    }

    // We failed to retrieve the port
    return {IpAddress(), 0};
}

template<class PtrType, long func(int, PtrType, size_t, int), void (TCPSocket::*waitMethod)()>
int TCPSocket::blockingOp(PtrType data, size_t size) {
    size_t processed = 0;
    while (processed != size) {
        (this->*waitMethod)();        
        long result = func(this->socket, data, size - processed, MSG_NOSIGNAL);
        // TODO: crossplatform wouldblock handling
        if (result != -1) {
            processed += static_cast<size_t>(result);
        } else if (getError() != EAGAIN && getError() != EWOULDBLOCK) {
            return getError();
        }
    }
    return 0;
}

template<class PtrType, long func(int, PtrType, size_t, int)>
int TCPSocket::nonblockingOp(PtrType data, size_t size, size_t & processed) {
    long result = ::send(this->socket, data, size, MSG_NOSIGNAL);
    if (result != -1) {
        processed = static_cast<size_t>(result);
        return 0;
    } else {
        return getError();
    }
}
