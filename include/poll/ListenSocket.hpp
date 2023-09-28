
#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include <string>

#include "Address.hpp"
#include "IFileDescriptor.hpp"

class ListenSocket : public IFileDescriptor {
 public:
  ~ListenSocket();
  void onPollEvent(struct pollfd &pollfd);
  static void create(Address const &addr, int backlog = SOMAXCONN);

 private:
  Address _addr;

  ListenSocket();
  ListenSocket(Address const &addr);
  ListenSocket(ListenSocket const &);
  ListenSocket &operator=(ListenSocket const &);
};

#endif  // LISTENSOCKET_HPP
