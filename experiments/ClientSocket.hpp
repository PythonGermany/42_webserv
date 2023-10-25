#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

#include "AConnection.hpp"
#include "Address.hpp"
#include "ListenSocket.hpp"

class ClientSocket : public AConnection {
 private:
  std::string _readBuffer;
  std::string _writeBuffer;

  Address _client;
  Address _host;

 public:
  ClientSocket(int fd, Address const &client, Address const &host);
  ~ClientSocket();

  void in();
  void out();
};

#endif
