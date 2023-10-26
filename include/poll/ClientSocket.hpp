#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include <algorithm>

#include "AConnection.hpp"
#include "Address.hpp"
#include "Http.hpp"
#include "ListenSocket.hpp"

class ClientSocket : public AConnection {
 private:
  std::string _readBuffer;

  char _writeBuffer[IO_OPERATION_SIZE];
  std::list<std::istream *> _writeStreams;

  Http _http;

 public:
  ClientSocket(int fd, Address const &client, Address const &host);
  ~ClientSocket();

  void in();
  void out();

  virtual void process();
};

#endif
