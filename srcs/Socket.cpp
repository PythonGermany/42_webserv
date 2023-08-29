#include "Socket.hpp"

#include <netdb.h>

Socket::Socket() : _fd(-1) {}

Socket::Socket(std::string host, std::string port, int max_clients) {
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  int status = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
  if (status != 0) throwException("getaddrinfo", gai_strerror(status));
  _fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (_fd < 0) throwException("socket", strerror(errno));

  // Bind and listen
  int result_bind = bind(_fd, result->ai_addr, result->ai_addrlen);
  if (result_bind < 0) throwException("bind", strerror(errno));
  freeaddrinfo(result);
  int result_listen = listen(_fd, max_clients);
  if (result_listen < 0) throwException("listen", strerror(errno));
}

Socket::Socket(const Socket& other) { *this = other; }

Socket& Socket::operator=(const Socket& other) {
  if (this == &other) return *this;
  _fd = other._fd;
  domain = other.domain;
  address = other.address;
  return *this;
}

Socket::~Socket() {}

int Socket::getPort() const { return _port; }

int Socket::fd() const { return _fd; }

void Socket::Setsockopt(int level, int option_name, const void* option_value,
                        socklen_t option_len) {
  int result = setsockopt(_fd, level, option_name, option_value, option_len);
  if (result < 0) throwException("setsockopt", strerror(errno));
}

void Socket::throwException(std::string func, std::string msg) const {
  throw std::runtime_error("Socket: " + func + ": " + msg);
}