#include "Socket.hpp"

Socket::Socket() : socket_fd(-1) {}

Socket::Socket(int domain, int type, int protocol) {
  socket_fd = socket(domain, type, protocol);
  this->domain = domain;
  if (socket_fd == -1) throw_error("socket");
}

Socket::Socket(const Socket& other) { *this = other; }

Socket& Socket::operator=(const Socket& other) {
  if (this == &other) return *this;
  socket_fd = other.socket_fd;
  domain = other.domain;
  address = other.address;
  return *this;
}

Socket::~Socket() {}

int Socket::fd() const { return socket_fd; }

void Socket::Setsockopt(int level, int option_name, const void* option_value,
                        socklen_t option_len) {
  int result =
      setsockopt(socket_fd, level, option_name, option_value, option_len);
  if (result < 0) throw_error("setsockopt");
}

void Socket::Bind(int port) {
  address.sin_family = domain;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(port);

  int result = bind(socket_fd, (sockaddr*)&address, sizeof(address));
  if (result < 0) throw_error("bind");
}

void Socket::Listen(int max_clients) {
  int result = listen(socket_fd, max_clients);
  if (result < 0) throw_error("listen");
}

void Socket::throw_error(const std::string& function_name) {
  std::string error = "socket: " + function_name + ": ";
  error += strerror(errno);
  throw std::runtime_error(error);
}