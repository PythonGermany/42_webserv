#include "Socket.hpp"

Socket::Socket() : socket_fd(-1) {}

Socket::Socket(int domain, int type, int protocol) {
  socket_fd = socket(domain, type, protocol);
  this->domain = domain;
  if (socket_fd == -1) {
    error = strerror(errno);
    throw std::runtime_error(error);
  }
}

Socket::Socket(const Socket& other) { *this = other; }

Socket& Socket::operator=(const Socket& other) {
  socket_fd = other.socket_fd;
  domain = other.domain;
  address = other.address;
  error = other.error;
  return *this;
}

Socket::~Socket() {}

int Socket::get_fd() const { return socket_fd; }

std::string Socket::get_error() const { return error; }

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
  error = function_name + ": ";
  error += strerror(errno);
  throw std::runtime_error(error);
}