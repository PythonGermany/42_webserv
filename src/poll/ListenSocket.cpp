#include "ListenSocket.hpp"

#include <fcntl.h>

#include "Http.hpp"
#include "Poll.hpp"

ListenSocket::ListenSocket() {}

ListenSocket::ListenSocket(Address const &addr) : _addr(addr) {}

/**
 * LisetnSocket::ListenSocket(Address const &) = delete;
 * ListenSocket &ListenSocket::operator=(ListenSocket const &) = delete;
 */

ListenSocket::~ListenSocket() {
  Log::write("Stop listening: " + toString<Address &>(_addr), DEBUG);
}

void ListenSocket::create(Address const &addr, int backlog) {
  struct pollfd pollfd;
  IFileDescriptor *newInstance;

  pollfd.fd = socket(addr.family(), SOCK_STREAM, 0);
  if (pollfd.fd < 0)
    throw std::runtime_error(std::string("ListenSocket(): socket(): ") +
                             std::strerror(errno));
  try {
    int flags = fcntl(pollfd.fd, F_GETFL, 0);
    if (flags == -1)
      throw std::runtime_error(std::string("ListenSocket(): fcntl(): ") +
                               std::strerror(errno));
    if (fcntl(pollfd.fd, F_SETFL, flags | O_NONBLOCK) == -1)
      throw std::runtime_error(std::string("ListenSocket(): fcntl(): ") +
                               std::strerror(errno));

    int reuse = 1;
    if (setsockopt(pollfd.fd, SOL_SOCKET, SO_REUSEADDR, &reuse,
                   sizeof(reuse)) == -1)
      throw std::runtime_error(std::string("ListenSocket(): setsockopt(): ") +
                               std::strerror(errno));

    if (bind(pollfd.fd, addr.data(), addr.size()))
      throw std::runtime_error(std::string("ListenSocket(): bind(): ") +
                               std::strerror(errno));

    if (listen(pollfd.fd, backlog))
      throw std::runtime_error(std::string("ListenSocket(): listen(): ") +
                               std::strerror(errno));

    newInstance = new ListenSocket(addr);

  } catch (...) {
    close(pollfd.fd);
    throw;
  }
  pollfd.events = POLLIN;
  pollfd.revents = 0;
  Poll::add(newInstance, pollfd);
}

void ListenSocket::onPollEvent(struct pollfd &pollfd) {
  struct pollfd newPollfd;
  socklen_t len = sizeof(sockaddr_in6);
  Address remoteAddress;
  Address serverAdresss;
  IFileDescriptor *CallbackObject;

  if ((pollfd.revents & POLLIN) == false) return;
  pollfd.revents &= ~POLLIN;
  newPollfd.fd = ::accept(pollfd.fd, remoteAddress.data(), &len);
  if (newPollfd.fd == -1)
    throw std::runtime_error(std::string("ListenSocket::onPollEvent(): ") +
                             std::strerror(errno));
  try {
    int flags = fcntl(newPollfd.fd, F_GETFL, 0);
    if (flags == -1)
      throw std::runtime_error(std::string("ListenSocket::onPollEvent(): ") +
                               std::strerror(errno));
    if (fcntl(newPollfd.fd, F_SETFL, flags | O_NONBLOCK) == -1)
      throw std::runtime_error(
          std::string("ListenSocket(): onPollIn(): fcntl(): ") +
          std::strerror(errno));
    remoteAddress.size(len);
    newPollfd.events = POLLIN;
    newPollfd.revents = 0;

    if (getsockname(newPollfd.fd, serverAdresss.data(), &len) == -1)
      throw std::runtime_error(std::string("ListenSocket::onPollEvent(): ") +
                               std::strerror(errno));
    serverAdresss.size(len);
    CallbackObject = new Http(remoteAddress, serverAdresss);
  } catch (...) {
    close(newPollfd.fd);
    throw;
  }
  Poll::add(CallbackObject, newPollfd);
}
