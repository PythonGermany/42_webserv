#ifndef HTTP_HPP
#define HTTP_HPP

#include <iostream>

#include "AConnection.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "VirtualHost.hpp"

class Http : public AConnection {
  Request _request;
  Response _response;
  VirtualHost *_virtualHost;

 public:
  Http(Address const &client, Address const &host);
  ~Http();

  void OnHeadRecv(std::string msg);
  void OnBodyRecv(std::string msg);
  void OnCgiRecv(std::string msg);
  void OnCgiTimeout();

 private:
  void processError();
};

#endif
