#include "../include/Address.hpp"

Address::Address()
{
    this->_family = 0;
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

    _addr.inet6.sin6_scope_id = 0;
    _addr.inet6.sin6_flowinfo = 0;
    if (inet_pton(AF_INET, tmp.c_str(), &_addr.inet.sin_addr) == 1)
    {
        this->_family = AF_INET;
        this->_addr.inet.sin_family = this->_family;
    }
    else if (inet_pton(AF_INET6, tmp.c_str(), &_addr.inet6.sin6_addr) == 1)
    {
        this->_family = AF_INET6;
        this->_addr.inet.sin_family = this->_family;
    }
    else
        throw std::invalid_argument("Address::Address(): invalid address: " + src);
}

Address::Address(Address const &other)  :  _addr(other._addr), _family(other._family) {}

Address &Address::operator=(Address const &other)
{
    _addr = other._addr;
    _family = other._family;
    return *this;
}

Address::~Address() {}

sa_family_t Address::family() const
{
    return this->_family;
}

void const *Address::addr() const
{
    if (this->_family == AF_INET)
        return &_addr.inet.sin_addr;
    else
        return &_addr.inet6.sin6_addr;
}

sockaddr *Address::data()
{
    return reinterpret_cast<sockaddr *>(&_addr);
}

sockaddr const *Address::data() const
{
    return reinterpret_cast<sockaddr const *>(&_addr);
}

socklen_t Address::size() const
{
    return this->_family == AF_INET ? sizeof(_addr.inet) : sizeof(_addr.inet6);
}

void Address::size(socklen_t size)
{
    if (size == sizeof(sockaddr_in))
        this->_family = AF_INET;
    else if (size == sizeof(sockaddr_in6))
        this->_family = AF_INET6;
    else
    {
        std::ostringstream oss;
        oss << size;
        throw std::invalid_argument("Address::size(): invalid size: " + oss.str());
    }
}

void Address::port(in_port_t port)
{
    port = htons(port);
    if (this->_family == AF_INET)
        _addr.inet.sin_port = port;
    else
        _addr.inet6.sin6_port = port;
}

in_port_t Address::port() const
{
    if (this->_family == AF_INET)
        return ntohs(_addr.inet.sin_port);
    else
        return ntohs(_addr.inet6.sin6_port);
}

/**
 * @throw std::runtime_error() if addr.family() is not AF_INET or AF_INET6
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

