
#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include <string>

#include "AConnection.hpp"
#include "Address.hpp"
#include "CallbackPointer.hpp"

class ListenSocket : public IFileDescriptor {
 public:
  ~ListenSocket();
  void onPollEvent(struct pollfd &pollfd, CallbackPointer *newCallbackObject,
                   struct pollfd *newPollfd);
  static void create(Address const &addr, int backlog = SOMAXCONN);

 private:
  Address _addr;
  size_t acceptAttempts;

  ListenSocket();
  ListenSocket(Address const &addr);
  ListenSocket(ListenSocket const &);
  ListenSocket &operator=(ListenSocket const &);
};

#endif  // LISTENSOCKET_HPP
