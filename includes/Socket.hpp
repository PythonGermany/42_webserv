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

  // Set socket options
  // @param level The level at which the option resides
  // @param option_name The name of the option to set
  // @param option_value The value to set the option to
  // @param option_len The length of the option value
  void Setsockopt(int level, int option_name, const void* option_value,
                  socklen_t option_len);

 private:
  // Throws an exeption using the format "Socket: [func]: [msg]"
  // @param func The function that threw the exeption
  // @param msg The message of the exeption
  // @exception std::runtime_error Thrown with the formatted message
  void throwException(std::string func, std::string msg) const;
};

#endif