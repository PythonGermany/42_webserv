#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

// Class to create and manage a socket
class Socket {
 private:
  int _port;
  int _fd;
  int domain;
  struct sockaddr_in address;

 public:
  Socket();
  Socket(std::string host, std::string port, int max_clients);
  Socket(const Socket& other);
  Socket& operator=(const Socket& other);
  ~Socket();

  // Getters
  int getPort() const;
  int fd() const;
  std::string get_error() const;

  void Setsockopt(int level, int option_name, const void* option_value,
                  socklen_t option_len);

 private:
  void throwException(std::string func, std::string msg) const;
};

#endif