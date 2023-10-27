#ifndef LISTENSOCKET_HPP
#define LISTENSOCKET_HPP

#include <fcntl.h>

#include "ClientSocket.hpp"
#include "Http.hpp"

#define BACKLOG 4096

class ClientSocket;

class ListenSocket : public AConnection {
 public:
  ListenSocket(Address const &addr);
  ~ListenSocket();

  void in();
  void out();
  void process();
};

#endif
