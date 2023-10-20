#include "Connection.hpp"

Connection::Connection(/* args */) {}

Connection::~Connection() {}

void Connection::read() {
  ssize_t len;
  char tmp[BUFFER_SIZE];

  if (_isSocket)
    len = ::recv(_readFd, tmp, BUFFER_SIZE, 0);
  else
    len = ::read(_readFd, tmp, BUFFER_SIZE);
  if (len == -1) _error = true;
  _readBuffer.append(tmp, len);
}

void Connection::write(const std::string &data) {
  ssize_t len;

  if (_isSocket)
    len = ::send(_readFd, data.c_str(), data.size(), 0);
  else
    len = ::write(_readFd, data.c_str(), data.size());
  if (len == -1) {
    _error = true;
    len = 0;
  }
  _writeBuffer.append(data.data() + len, data.size() - len);
}