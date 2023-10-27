#include "ClientSocket.hpp"

ClientSocket::ClientSocket(int fd, Address const &client, Address const &host)
    : AConnection(fd, true, false), _http(client, host) {}

ClientSocket::~ClientSocket() {}

void ClientSocket::in() {
  char buffer[IO_OPERATION_SIZE];
  int ret = recv(_fd, buffer, IO_OPERATION_SIZE, 0);
  if (ret < 0)
    setStateBits(ERROR);
  else if (ret == 0)
    setStateBits(CLOSED);
  else
    _readBuffer.append(buffer, ret);
}

void ClientSocket::out() {
  // size_t toSend = std::min(IO_OPERATION_SIZE, _writeBuffer.size());
  // int ret = send(_fd, _writeBuffer.data(), toSend, 0);
  // if (ret == -1)
  //   setStateBits(ERROR);
  // else {
  //   _writeBuffer.erase(0, ret);
  //   _listenOut = _writeBuffer.empty();
  // }

  std::istream *stream = _writeStreams.front();

  stream->read(_writeBuffer, IO_OPERATION_SIZE);
  size_t toSend = stream->gcount();
  if (stream->eof()) {
    delete _writeStreams.front();
    _writeStreams.pop_front();
  } else if (stream->fail())
    throw std::runtime_error("AConnection::onPollOut(): std::istream.read()");

  ssize_t ret = send(_fd, _writeBuffer, toSend, 0);
  if (ret == -1) setStateBits(ERROR);
  if (_writeStreams.empty()) {
    _listenOut = false;
    if (_http.keepAlive() == false) setStateBits(DONE);
  }
}

void ClientSocket::process() {
  _http.process(_readBuffer);
  std::list<std::istream *> &output = _http.output();
  while (output.size() > 0) {
    _writeStreams.push_back(output.front());
    output.pop_front();
  }
  _listenOut = _writeStreams.size() > 0;
}
