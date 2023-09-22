#include "Address.hpp"

Address::Address() { this->_family = 0; }

Address::Address(std::string const &src, std::string const &port) {
  struct addrinfo hints, *res;

  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_socktype = SOCK_STREAM;

  _addr.inet6.sin6_scope_id = 0;
  _addr.inet6.sin6_flowinfo = 0;
  if (getaddrinfo(src.c_str(), port.c_str(), &hints, &res) != 0)
    throw std::invalid_argument("Address::Address(): invalid address: " + src +
                                ":" + port);
  if (res->ai_family == AF_INET) {
    this->_family = AF_INET;
    this->_addr.inet.sin_family = res->ai_family;
    this->_addr.inet.sin_family = this->_family;
    std::memcpy(&this->_addr.inet, res->ai_addr, res->ai_addrlen);
  } else if (res->ai_family == AF_INET6) {
    this->_family = AF_INET6;
    this->_addr.inet6.sin6_family = res->ai_family;
    std::memcpy(&this->_addr.inet6, res->ai_addr, res->ai_addrlen);
  } else {
    freeaddrinfo(res);
    throw std::invalid_argument("Address::Address(): invalid address: " + src +
                                ":" + port);
  }
  freeaddrinfo(res);
}

Address::Address(Address const &other)
    : _addr(other._addr), _family(other._family) {}

Address &Address::operator=(Address const &other) {
  _addr = other._addr;
  _family = other._family;
  return *this;
}

Address::~Address() {}

sa_family_t Address::family() const { return this->_family; }

void Address::family(sa_family_t family) {
  if (family == AF_INET || family == AF_INET6)
    this->_family = family;
  else {
    std::ostringstream oss;
    oss << family;
    throw std::invalid_argument("Address::family(): invalid family: " +
                                oss.str());
  }
}

void const *Address::addr() const {
  if (this->_family == AF_INET)
    return &_addr.inet.sin_addr;
  else
    return &_addr.inet6.sin6_addr;
}

sockaddr *Address::data() { return reinterpret_cast<sockaddr *>(&_addr); }

sockaddr const *Address::data() const {
  return reinterpret_cast<sockaddr const *>(&_addr);
}

socklen_t Address::size() const {
  return this->_family == AF_INET ? sizeof(_addr.inet) : sizeof(_addr.inet6);
}

void Address::size(socklen_t size) {
  if (size == sizeof(sockaddr_in))
    this->_family = AF_INET;
  else if (size == sizeof(sockaddr_in6))
    this->_family = AF_INET6;
  else {
    std::ostringstream oss;
    oss << size;
    throw std::invalid_argument("Address::size(): invalid size: " + oss.str());
  }
}

void Address::port(in_port_t port) {
  port = htons(port);
  if (this->_family == AF_INET)
    _addr.inet.sin_port = port;
  else
    _addr.inet6.sin6_port = port;
}

in_port_t Address::port() const {
  if (this->_family == AF_INET)
    return ntohs(_addr.inet.sin_port);
  else
    return ntohs(_addr.inet6.sin6_port);
}

// Start pythongermany code
bool Address::operator==(Address const &other) const {
  if (this->_family != other._family) return false;
  if (this->_family == AF_INET) {
    if (this->_addr.inet.sin_port != other._addr.inet.sin_port) return false;
    return std::memcmp(&this->_addr.inet.sin_addr, &other._addr.inet.sin_addr,
                       sizeof(this->_addr.inet.sin_addr)) == 0;
  } else {
    if (this->_addr.inet6.sin6_port != other._addr.inet6.sin6_port)
      return false;
    return std::memcmp(&this->_addr.inet6.sin6_addr,
                       &other._addr.inet6.sin6_addr,
                       sizeof(this->_addr.inet6.sin6_addr)) == 0;
  }
}

/**
 * @throw std::runtime_error() if addr.family() is not AF_INET or AF_INET6
 */
std::ostream &operator<<(std::ostream &os, Address const &addr) {
  char buffer[INET6_ADDRSTRLEN];

  if (!inet_ntop(addr.family(), addr.addr(), buffer, INET6_ADDRSTRLEN))
    throw std::runtime_error(std::string("<<Address: ") + std::strerror(errno));
  if (addr.family() == AF_INET) {
    os << buffer << ":" << addr.port();
  } else {
    os << "[" << buffer << "]:" << addr.port();
  }
  return os;
}
