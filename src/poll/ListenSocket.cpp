#include "ListenSocket.hpp"

#include <fcntl.h>

#include "Http.hpp"
#include "Poll.hpp"
#include "global.hpp"
#include "utils.hpp"

ListenSocket::ListenSocket(Address const &addr)
    : _addr(addr), acceptAttempts(0) {}

/**
 * LisetnSocket::ListenSocket(Address const &) = delete;
 * ListenSocket &ListenSocket::operator=(ListenSocket const &) = delete;
 */

ListenSocket::~ListenSocket() {
  accessLog_g.write("Stop listening: " + toString<Address &>(_addr), DEBUG);
}

void ListenSocket::create(Address const &addr, int backlog) {
  struct pollfd pollfd;
  CallbackPointer newInstance;

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

    newInstance.link = false;
    newInstance.ptr = new ListenSocket(addr);
  } catch (...) {
    close(pollfd.fd);
    throw;
  }
  pollfd.events = POLLIN;
  pollfd.revents = 0;
  accessLog_g.write("Listen: " + toString<Address const &>(addr), DEBUG);
  Poll::add(newInstance, pollfd);
}

void ListenSocket::onPollEvent(struct pollfd &pollfd,
                               CallbackPointer *newCallbackObject,
                               struct pollfd *newPollfd) {
  socklen_t len = sizeof(sockaddr_in6);
  Address remoteAddress;
  Address serverAdresss;
  if ((pollfd.revents & POLLIN) == false && pollfd.revents)
    throw std::runtime_error("POLLERR");
  if ((pollfd.revents & POLLIN) == false) return;
  pollfd.revents &= ~POLLIN;
  newPollfd->fd = ::accept(pollfd.fd, remoteAddress.data(), &len);
  if (newPollfd->fd == -1)
    throw std::runtime_error(std::string("ListenSocket::onPollEvent(): ") +
                             std::strerror(errno));
  int flags = fcntl(newPollfd->fd, F_GETFL, 0);
  if (flags == -1)
    throw std::runtime_error(std::string("ListenSocket::onPollEvent(): ") +
                             std::strerror(errno));
  if (fcntl(newPollfd->fd, F_SETFL, flags | O_NONBLOCK) == -1)
    throw std::runtime_error(
        std::string("ListenSocket(): onPollIn(): fcntl(): ") +
        std::strerror(errno));
  remoteAddress.size(len);
  newPollfd->events = POLLIN;
  newPollfd->revents = 0;

  if (getsockname(newPollfd->fd, serverAdresss.data(), &len) == -1)
    throw std::runtime_error(std::string("ListenSocket::onPollEvent(): ") +
                             std::strerror(errno));

  serverAdresss.size(len);
  newCallbackObject->link = false;
  try {
    newCallbackObject->ptr = new Http(remoteAddress, serverAdresss);
  } catch (std::bad_alloc const &e) {
    close(newPollfd->fd);
    newPollfd->fd = -1;
    try {
      errorLog_g.write("Failed to accept client: ENOMEM", DEBUG, BRIGHT_RED);
    } catch (...) {
    }
  }
}
