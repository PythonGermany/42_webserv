#ifndef HTTP_HPP
#define HTTP_HPP

#include <fstream>
#include <iostream>
#include <sstream>

#include "AConnection.hpp"
#include "Log.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "VirtualHost.hpp"

class Http : public AConnection {
  Request _request;
  Response _response;
  VirtualHost* _virtualHost;
  Context* _context;
  bool _waitForBody;

 public:
  Http(Address const& client, Address const& host);
  ~Http();

  void OnHeadRecv(std::string msg);
  void OnBodyRecv(std::string msg);
  void OnCgiRecv(std::string msg);
  void OnCgiTimeout();

 private:
  Response& processRequest();
  Response& processFile(std::string path);
  Response& processAutoindex(std::string path);
  Response& processRedirect(std::string path);
  Response& processError(std::string code, std::string reason);

  std::string getDefaultBody(std::string code, std::string reason);
};

#endif
