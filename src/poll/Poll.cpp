#include "Poll.hpp"

Poll::Poll() : _timeout(-1) {}

Poll::~Poll() {
  for (size_t i = 0; i < _connections.size(); i++) delete _connections[i];
}

int Poll::add(AConnection *connection) {
  if (connection == NULL) return 0;
  try {
    pollfd newPollFd = {connection->fd(), 0, 0};
    _pollfds.push_back(newPollFd);
    try {
      _connections.push_back(connection);
    } catch (const std::exception &e) {
      _pollfds.pop_back();
      throw std::runtime_error("Failed add connection to poll");
    }
  } catch (...) {
    return 1;
  }
  return 0;
}

void Poll::remove(size_t i) {
  delete _connections[i];
  _connections.erase(_connections.begin() + i);
  _pollfds.erase(_pollfds.begin() + i);
}

void Poll::updateRequestedEvents() {
  for (size_t i = 0; i < _connections.size(); i++) {
    _pollfds[i].events = 0;
    if (_connections[i]->listenIn()) _pollfds[i].events |= POLLIN;
    if (_connections[i]->listenOut()) _pollfds[i].events |= POLLOUT;
  }
}

int Poll::update() {
  updateRequestedEvents();

  int ret = poll(_pollfds.data(), _pollfds.size(), _timeout);
  if (ret == -1) {
    errorLog_g.write("Poll: Fatal error", ERROR);
    return 1;
  } else if (ret == 0)
    accessLog_g.write("Poll: No poll event", DEBUG, YELLOW);
  else {
    processConnectionEvents();
    addConnectionQueue();
    removeStaleConnections();
  }
  return 0;
}

void Poll::processConnectionEvents() {
  for (size_t i = 0; i < _connections.size(); i++) {
    try {
      if (_pollfds[i].revents & (POLLERR | POLLHUP)) {
        accessLog_g.write("Poll: POLLERR or POLLHUP", DEBUG, YELLOW);
        throw std::exception();
      } else {
        if (_pollfds[i].revents & POLLIN) _connections[i]->in();
        if (_pollfds[i].revents & POLLOUT) _connections[i]->out();
        if (_connections[i]->remove() == false) _connections[i]->process();
      }
    } catch (...) {
      _connections[i]->setStateBits(AConnection::ERROR);
    }
    _pollfds[i].revents = 0;
  }
}

void Poll::addConnectionQueue() {
  AConnection *connection = AConnection::popQueueFront();
  while (connection != NULL) {
    if (add(connection)) {
      delete connection;
      std::cerr << "Failed to add connection to poll\n";
    }
    connection = AConnection::popQueueFront();
  }
}

void Poll::removeStaleConnections() {
  for (size_t i = 0; i < _connections.size(); i++)
    if (_connections[i]->remove()) remove(i--);
}
