
#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include <string>

#include "Address.hpp"
#include "IFileDescriptor.hpp"

class ListenSocket : public IFileDescriptor {
 public:
  ListenSocket(std::string const &addr, std::string const &port,
               int backlog = SOMAXCONN);
  ListenSocket(Address const &addr, int backlog = SOMAXCONN);
  ListenSocket(ListenSocket const &other);
  ~ListenSocket();
  ListenSocket &operator=(ListenSocket const &other);
  void onPollEvent(struct pollfd &pollfd);

 private:
  Address _addr;

  ListenSocket();
  void init(int backlog);
};

#endif  // LISTENSOCKET_HPP
