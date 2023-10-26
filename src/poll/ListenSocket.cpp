#include "ListenSocket.hpp"

ListenSocket::ListenSocket(Address const &addr) : AConnection(-1, true, false) {
  try {
    _fd = socket(addr.family(), SOCK_STREAM, 0);
    if (_fd == -1) throw std::runtime_error("Failed to create socket");

    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1 || fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
      throw std::runtime_error("Failed to set socket options");

    int reuse = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
      throw std::runtime_error("Failed to set socket options");

    if (bind(_fd, addr.data(), addr.size()))
      throw std::runtime_error("Failed to bind socket");
    if (listen(_fd, BACKLOG))
      throw std::runtime_error("Failed to prepare socket");
  } catch (const std::exception &e) {
    setStateBits(ERROR);
    throw std::runtime_error(e.what());
  }
}

ListenSocket::~ListenSocket() {}

void ListenSocket::in() {
  socklen_t len = sizeof(sockaddr_in6);
  Address clientAddress;
  Address serverAdresss;

  int clientFd = ::accept(_fd, clientAddress.data(), &len);
  if (clientFd == -1) return;

  ClientSocket *client = NULL;
  try {
    int flags = fcntl(clientFd, F_GETFL, 0);
    if (flags == -1 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1)
      throw std::runtime_error("Failed to set socket options");
    clientAddress.size(len);

    if (getsockname(clientFd, serverAdresss.data(), &len) == -1)
      throw std::runtime_error("Failed to get socket address");
    serverAdresss.size(len);

    client = new ClientSocket(clientFd, clientAddress, serverAdresss);
    if (pushQueueFront(client))
      throw std::runtime_error("Failed to add client to queue");
  } catch (...) {
    delete client;
    close(clientFd);
    return;
  }
}

void ListenSocket::out() {}

void ListenSocket::process() {}
