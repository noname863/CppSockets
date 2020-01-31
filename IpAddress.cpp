#include "IpAddress.hpp"
// TODO switch headers if windows
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>

IpAddress::IpAddress(std::string address) {
    *this = address;
}

IpAddress::IpAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
    // m_address would be in network order
    this->m_address = (byte0) | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);
    this->m_valid = true;
}

IpAddress::IpAddress(uint32_t address) {
    *this = address;    
}

IpAddress & IpAddress::operator=(const uint32_t & address) {
    this->m_address = htonl(address);
    return *this;
}

IpAddress & IpAddress::operator=(const std::string & address) {
    this->m_address = 0;
    this->m_valid = false;
    if (address == "255.255.255.255") {
        // The broadcast address needs to be handled explicitly,
        // because it is also the value returned by inet_addr on error
        this->m_address = INADDR_BROADCAST;
        this->m_valid = true;        
    } else if (address == "0.0.0.0"){
        this->m_address = INADDR_ANY;
        this->m_valid = true;        
    } else {
        // parce address
        uint32_t ip = inet_addr(address.c_str());
        if (ip != INADDR_NONE) {
            this->m_address = ip;
            this->m_valid = true;
        } else {
            // if it not address, try to resolve hostname
            addrinfo hints;
            std::memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            addrinfo* result = NULL;
            if (getaddrinfo(address.c_str(), NULL, &hints, &result) == 0)
            {
                if (result)
                {
                    ip = reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
                    freeaddrinfo(result);
                    this->m_address = ip;
                    this->m_valid = true;
                }
            }
        }
    }
    return *this;
}

IpAddress::operator uint32_t() const {
    return this->m_address;
}

bool IpAddress::isValid() {
    return this->m_valid;
}


