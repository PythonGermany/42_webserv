#include "Address.hpp"

Address::Address() { this->_family = 0; }

Address::Address(sa_family_t family, in_port_t port) {
  std::ostringstream oss;

  if (family == AF_INET) {
    _addr.inet.sin_addr.s_addr = INADDR_ANY;
    _addr.inet.sin_family = AF_INET;
    _addr.inet.sin_port = htons(port);
    _family = AF_INET;
  } else if (family == AF_INET6) {
    _addr.inet6.sin6_addr = in6addr_any;
    _addr.inet6.sin6_family = AF_INET6;
    _addr.inet6.sin6_port = htons(port);
    _family = AF_INET6;
  } else {
    oss << _family;
    throw std::invalid_argument("Address::Address(): invalid address family: " +
                                oss.str());
  }
}

Address::Address(sa_family_t family, sockaddr *src) {
  std::ostringstream oss;

  _family = family;
  if (_family == AF_INET)
    _addr.inet = *reinterpret_cast<sockaddr_in *>(src);
  else if (_family == AF_INET6)
    _addr.inet6 = *reinterpret_cast<sockaddr_in6 *>(src);
  else {
    oss << _family;
    throw std::invalid_argument("Address::Address(): invalid address family: " +
                                oss.str());
  }
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

bool Address::operator<(Address const &other) const {
  int diff_addr;
  int diff_port;

  if (_family != other._family) {
    return _family == AF_INET ? true : false;
  }
  if (_family == AF_INET) {
    diff_addr = std::memcmp(&_addr.inet.sin_addr, &other._addr.inet.sin_addr,
                            sizeof(_addr.inet.sin_addr));
    diff_port = std::memcmp(&_addr.inet.sin_port, &other._addr.inet.sin_port,
                            sizeof(_addr.inet.sin_port));
  } else {
    diff_addr =
        std::memcmp(&_addr.inet6.sin6_addr, &other._addr.inet6.sin6_addr,
                    sizeof(_addr.inet6.sin6_addr));
    diff_port =
        std::memcmp(&_addr.inet6.sin6_port, &other._addr.inet6.sin6_port,
                    sizeof(_addr.inet6.sin6_port));
  }
  if (diff_addr < 0) return true;
  if (diff_port < 0) return true;
  return false;
}

static sa_family_t splitHost(std::string const &src, std::string &hostname,
                             std::string &port) {
  size_t seperator = src.rfind(':');

  if (seperator == std::string::npos) {
    hostname = "0.0.0.0";
    port = src;
    return AF_INET;
  }
  hostname = src.substr(0, seperator);
  port = src.substr(seperator + 1);
  if (hostname.size() && hostname[0] == '[' && *(hostname.end() - 1) == ']') {
    hostname = std::string(hostname.begin() + 1, hostname.end() - 1);
    return AF_INET6;
  } else
    return AF_UNSPEC;
}

std::set<Address> Address::resolveHost(std::string const &src) {
  std::set<Address> set;
  struct addrinfo hints = {};
  struct addrinfo *result = NULL;
  struct addrinfo *list;
  int ret;
  std::string hostname;
  std::string port;

  hints.ai_family = splitHost(src, hostname, port);
  hints.ai_socktype = SOCK_STREAM;
  ret = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &result);
  if (ret != 0) {
    if (result) freeaddrinfo(result);
    if (ret == EAI_SYSTEM)
      throw std::runtime_error("Address::resolveHost: " + src + ": " +
                               std::strerror(errno));
    else
      throw std::runtime_error("Address::resolveHost: " + src + ": " +
                               gai_strerror(ret));
  }
  list = result;
  while (list) {
    set.insert(Address(list->ai_family, list->ai_addr));
    list = list->ai_next;
  }
  if (result)
    freeaddrinfo(result);
  else
    throw std::invalid_argument("Address::resolveHost: " + src);
  return set;
}

bool Address::operator==(Address const &other) const {
  if (_family != other._family) return false;
  if (_family == AF_INET) {
    if (_addr.inet.sin_port != other._addr.inet.sin_port) return false;
    return std::memcmp(&_addr.inet.sin_addr, &other._addr.inet.sin_addr,
                       sizeof(_addr.inet.sin_addr)) == 0;
  } else {
    if (_addr.inet6.sin6_port != other._addr.inet6.sin6_port) return false;
    return std::memcmp(&_addr.inet6.sin6_addr, &other._addr.inet6.sin6_addr,
                       sizeof(_addr.inet6.sin6_addr)) == 0;
  }
}

static std::ostream &operator<<(std::ostream &os, in_addr const &src) {
  int32_t hostByteOrder = ntohl(src.s_addr);
  os << (hostByteOrder >> 24 & 0xFF) << '.';
  os << (hostByteOrder >> 16 & 0xFF) << '.';
  os << (hostByteOrder >> 8 & 0xFF) << '.';
  os << (hostByteOrder & 0xFF);
  return os;
}

static std::pair<unsigned int, unsigned int> allZeroFields(
    unsigned char const *addr) {
  unsigned int currentStart = 0;
  unsigned int currentSize = 0;
  std::pair<unsigned int, unsigned int> result;
  unsigned int bestSize = 0;

  for (unsigned int i = 0; i < 8; ++i) {
    unsigned int pos = i * 2;
    if (addr[pos] != 0 || addr[pos + 1] != 0) {
      currentSize = 0;
      continue;
    }
    if (currentSize == 0) {
      currentSize = 1;
      currentStart = pos;
    } else {
      ++currentSize;
    }
    if (currentSize > bestSize) {
      bestSize = currentSize;
      result.first = currentStart;
    }
  }

  if (bestSize) {
    result.second = result.first + bestSize * 2;
    result.second -= 1;
    if (result.second - result.first == 1) result.second = 0;
  }
  return result;
}

static std::ostream &operator<<(std::ostream &os, in6_addr const &src) {
  unsigned char const *addr = reinterpret_cast<unsigned char const *>(&src);

  os << '[';
  if ((IN6_IS_ADDR_V4COMPAT(addr) && (addr[12] != 0 || addr[13] != 0)) ||
      IN6_IS_ADDR_V4MAPPED(addr)) {
    addr[10] == 255 ? os << "::ffff:" : os << "::";
    os << *reinterpret_cast<in_addr const *>(&addr[12]);
    os << ']';
    return os;
  }

  std::pair<unsigned int, unsigned int> zeroField = allZeroFields(addr);
  std::ios_base::fmtflags flags =
      os.setf(std::ios_base::hex, std::ios_base::basefield);

  for (size_t i = 0; i < 16; ++i) {
    bool first = (i % 2 == 0);

    if (zeroField.second == 0 || i < zeroField.first || i > zeroField.second) {
      if (!first) {
        if (addr[i - 1] != 0) os << std::setw(2) << std::setfill('0');
        os << (int)addr[i];
        if (i != 15) os << ':';
      } else if (addr[i] != 0) {
        os << (int)addr[i];
      }
    } else if (i == zeroField.second || (i == 0 && zeroField.first == 0)) {
      os << ':';
    }
  }
  os.flags(flags);
  os << ']';
  return os;
}

std::ostream &operator<<(std::ostream &os, Address const &addr) {
  if (addr.family() == AF_INET6)
    os << *reinterpret_cast<in6_addr const *>(addr.addr());
  else
    os << *reinterpret_cast<in_addr const *>(addr.addr());
  os << ':' << addr.port();
  return os;
}
