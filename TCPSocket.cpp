#include "TCPSocket.hpp"
// TODO: add windows headers
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>

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
        this->setNagle(nagleEnabled);
        this->closed = false;
    }
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
            if (getRemoteAddress() == IpAddress()) {
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
    return ::close(this->socket);
}

bool TCPSocket::isConnected() {
    return connected;
}

IpAddress TCPSocket::getRemoteAddress() {
    if (!this->closed) {
        sockaddr_in address;
        socklen_t size = sizeof(address);
        if (::getsockname(this->socket, reinterpret_cast<sockaddr *>(&address), &size) != -1) {
            return address.sin_addr.s_addr;
        }
    }
    return IpAddress();
}
