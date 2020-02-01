#include "TCPSocket.hpp"
// TODO: add windows headers
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

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

int TCPSocket::getError() {
    return errno;
}

TCPSocket::TCPSocket() {
    create();
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
        return -1;
    } else {
        if (blocking) {
            waitWriteReady(std::forward<waitArgs>(args)...);
            // check if connected            
            if (getRemoteAddressPort().first == IpAddress()) {
                return -1;
            }
        }
        connected = true;
    }
    return 0;
}

int TCPSocket::connect(const IpAddress & address, uint16_t port)
{
    return connectImpl(address, port);
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
        int result = ::send(this->socket, data, size, MSG_NOSIGNAL);
        if (result != -1) {
            return 0;
        } else {
            return getError();
        }
    }
    size_t localSent = 0;
    while (localSent != size) {
        int result = ::send(this->socket, data, size - localSent, MSG_NOSIGNAL);
        // TODO: crossplatform wouldblock handling
        if (result != -1 || getError() == EAGAIN || getError() == EWOULDBLOCK) {
            localSent += result;
        } else {
            return getError();
        }
        waitWriteReady();
    }
    return 0;
}

int TCPSocket::send(const void * data, size_t size, size_t & sent) {
    if (!this->blocking) {
        // TODO: change flags not in linux
        int result = ::send(this->socket, data, size, MSG_NOSIGNAL);
        if (result != -1) {
            sent = result;
            return 0;
        } else {
            return getError();
        }
    }
    sent = size;
    return this->send(data, size);
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


