#include "ClientSocket.hpp"

ClientSocket::ClientSocket(int fd, Address const &client, Address const &host)
    : AConnection(fd, true, false) {
  (void)client;
  (void)host;
}

ClientSocket::~ClientSocket() {}

void ClientSocket::in() {
  char buffer[BUFFER_SIZE];
  int ret = recv(_fd, buffer, BUFFER_SIZE, 0);
  if (ret <= 0)
    setStateBits(ERROR);
  else
    _readBuffer.append(buffer, ret);
}

void ClientSocket::out() {
  int ret = send(_fd, _writeBuffer.data(), BUFFER_SIZE, 0);
  if (ret == -1)
    setStateBits(ERROR);
  else {
    _writeBuffer.erase(0, ret);
    _listenOut = _writeBuffer.empty();
  }
}

void ClientSocket::process() {}
