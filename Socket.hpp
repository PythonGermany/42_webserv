#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

class Socket {
 private:
  int socket_fd;
  int domain;
  struct sockaddr_in address;
  std::string error;

 public:
  Socket();
  Socket(int domain, int type, int protocol);
  Socket(const Socket& other);
  Socket& operator=(const Socket& other);
  ~Socket();

  int get_fd() const;
  std::string get_error() const;
  void Setsockopt(int level, int option_name, const void* option_value,
                  socklen_t option_len);
  void Bind(int port);
  void Listen(int max_clients);

 private:
  void throw_error(const std::string& function_name);
};

#endif