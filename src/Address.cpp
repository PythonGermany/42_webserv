#include "../include/Address.hpp"

Address::Address() {}

/**
 * @throw std::runtime_error() if src is not valid ipv4/ipv6 address
*/
Address::Address(char const *const src)
{
    ip.v6.sin6_scope_id = 0;
    ip.v6.sin6_flowinfo = 0;
    if (inet_pton(AF_INET, src, &ip.v4.sin_addr) == 1)
        data()->sa_family = AF_INET;
    else if (inet_pton(AF_INET6, src, &ip.v6.sin6_addr) == 1)
        data()->sa_family = AF_INET6;
    else
        throw std::runtime_error(std::string("Address(): invalid address: ") + src);
}

Address::Address(Address const &other) : ip(other.ip) {}

Address &Address::operator=(Address const &other)
{
    ip = other.ip;
    return *this;
}

Address::~Address() {}

sa_family_t Address::family() const
{
    return data()->sa_family;
}

void const *Address::addr() const
{
    if (family() == AF_INET)
        return &ip.v4.sin_addr;
    else
        return &ip.v6.sin6_addr;
}

sockaddr *Address::data()
{
    return reinterpret_cast<sockaddr *>(&ip);
}

sockaddr const *Address::data() const
{
    return reinterpret_cast<sockaddr const *>(&ip);
}

socklen_t Address::size() const
{
    return family() == AF_INET ? sizeof(ip.v4) : sizeof(ip.v6);
}

void Address::port(in_port_t port)
{
    port = htons(port);
    if (family() == AF_INET)
        ip.v4.sin_port = port;
    else
        ip.v6.sin6_port = port;
}

in_port_t Address::port() const
{
    if (family() == AF_INET)
        return ntohs(ip.v4.sin_port);
    else
        return ntohs(ip.v6.sin6_port);
}

/**
 * @throw std::rutnimer_error() if addr.family() is not AF_INET or AF_INET6
*/
std::ostream &operator<<(std::ostream &os, Address const &addr)
{
    char buffer[INET6_ADDRSTRLEN];

    if (!inet_ntop(addr.family(), addr.addr(), buffer, INET6_ADDRSTRLEN))
        throw std::runtime_error(std::string("<<Address: ") + std::strerror(errno));
    if (addr.family() == AF_INET)
    {
        os << buffer << ":" << addr.port();
    }
    else
    {
        os << "[" << buffer << "]:" << addr.port();
    }
    return os;
}

