#ifndef POLL_HPP
#define POLL_HPP

#include <poll.h>

#include <iostream>
#include <vector>

#include "AConnection.hpp"

class Poll {
 private:
  std::vector<AConnection *> _connections;
  std::vector<struct pollfd> _pollfds;
  int _timeout;

 public:
  Poll();
  ~Poll();

  int add(AConnection *connection);
  void remove(size_t i);
  int update();

 private:
  void updateRequestedEvents();
  void processOccuredEvents();
  void addConnectionQueue();
  void destroyFailedConnections();
};

#endif
