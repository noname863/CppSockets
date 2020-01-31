#ifndef IPADDRESS_HPP
#define IPADDRESS_HPP
#include <string>
#include <cstdint>

class IpAddress {
    uint32_t m_address;  
    bool m_valid = false;
public:
    IpAddress() = default;
    IpAddress(std::string address);
    IpAddress(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);
    IpAddress(uint32_t address);
    operator uint32_t() const;
    IpAddress & operator=(const uint32_t & address);
    IpAddress & operator=(const std::string & address);
    bool isValid();
};

#endif // IPADDRESS_HPP
