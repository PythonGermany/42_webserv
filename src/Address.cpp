#include "../include/Address.hpp"

Address::Address()
{
    this->data()->sa_family = 0;
}

Address::Address(std::string const &src, std::string const &port)
{
    std::string         tmp(src);
    std::stringstream   ss;
    int32_t            _p;

    if (port.empty() || std::isspace(port[0]))
        throw std::invalid_argument("Address::Address(): invalid port: " + port);
    ss << port;
    ss >> _p;
    if (!ss.fail() && ss.eof() && _p >= 0 && _p <= std::numeric_limits<in_port_t>::max())
        this->port(_p);
    else
        throw std::invalid_argument("Address::Address(): invalid port: " + port);

    if (tmp.empty())
        tmp = "0.0.0.0";
    std::string::iterator end = tmp.end() - 1;
    std::string::iterator start = tmp.begin();
    if (*end == ']' && *start == '[')
        tmp = std::string(start + 1, end);

    _inet6.sin6_scope_id = 0;
    _inet6.sin6_flowinfo = 0;
    if (inet_pton(AF_INET, tmp.c_str(), &_inet.sin_addr) == 1)
        this->data()->sa_family = AF_INET;
    else if (inet_pton(AF_INET6, tmp.c_str(), &_inet6.sin6_addr) == 1)
        this->data()->sa_family = AF_INET6;
    else
        throw std::invalid_argument("Address::Address(): invalid address: " + src);
}

Address::Address(Address const &other)  :   _inet6(other._inet6) {}

Address &Address::operator=(Address const &other)
{
    _inet6 = other._inet6;
    return *this;
}

Address::~Address() {}

sa_family_t Address::family() const
{
    return this->data()->sa_family;
}

void const *Address::addr() const
{
    if (this->family() == AF_INET)
        return &_inet.sin_addr;
    else
        return &_inet6.sin6_addr;
}

sockaddr *Address::data()
{
    return reinterpret_cast<sockaddr *>(&_inet);
}

sockaddr const *Address::data() const
{
    return reinterpret_cast<sockaddr const *>(&_inet);
}

socklen_t Address::size() const
{
    return this->family() == AF_INET ? sizeof(_inet) : sizeof(_inet6);
}

void Address::port(in_port_t port)
{
    port = htons(port);
    if (this->family() == AF_INET)
        _inet.sin_port = port;
    else
        _inet6.sin6_port = port;
}

in_port_t Address::port() const
{
    if (this->family() == AF_INET)
        return ntohs(_inet.sin_port);
    else
        return ntohs(_inet6.sin6_port);
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

