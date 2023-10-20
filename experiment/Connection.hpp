#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <sys/socket.h>
#include <unistd.h>

#include <string>

#define BUFFER_SIZE 65536

class Connection {
 private:
  int _readFd;
  int _writeFd;
  bool _isSocket;
  bool _error;

  std::string _readBuffer;
  std::string _writeBuffer;

 public:
  Connection(/* args */);
  ~Connection();

  void read();
  void write(const std::string &data);
};

#endif