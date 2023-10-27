#include "AConnection.hpp"

std::list<AConnection *> AConnection::_queue;

AConnection::AConnection()
    : _fd(-1), _listenIn(false), _listenOut(false), _state(0) {}

AConnection::AConnection(int fd, bool listenIn, bool listenOut)
    : _fd(fd), _listenIn(listenIn), _listenOut(listenOut), _state(0) {}

AConnection::~AConnection() {
  if (_fd >= 0) close(_fd);
}

int AConnection::pushQueueBack(AConnection *connection) {
  try {
    if (connection != NULL) _queue.push_back(connection);
  } catch (...) {
    return 1;
  }
  return 0;
}

AConnection *AConnection::popQueueFront() {
  if (_queue.empty()) return NULL;
  AConnection *ret = _queue.front();
  _queue.pop_front();
  return ret;
}

int AConnection::fd() { return _fd; }

bool AConnection::listenIn() { return _listenIn; }

bool AConnection::listenOut() { return _listenOut; }

void AConnection::setStateBits(state_t bits) { _state |= bits; }

void AConnection::clearStateBits(state_t bits) { _state &= ~bits; }

bool AConnection::getStateBit(state_t bit) { return _state & bit; }

bool AConnection::remove() { return _state; }
