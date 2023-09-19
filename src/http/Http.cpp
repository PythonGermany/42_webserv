#include "Http.hpp"

Http::Http(Address const &client, Address const &host) {
  this->client = client;
  this->host = host;
  this->msgdelimiter = "\r\n\r\n";
  this->msgsizelimit = 10000;
  this->msgsize = 10000;
  std::cout << "create: " << client << std::endl;
}

Http::~Http() { std::cout << "delete: " << client << std::endl; }

void Http::OnHeadRecv(std::string msg) {
  std::cout << "$$$$$$$$$ BEGIN HEAD $$$$$$$$$$ >";
  std::cout << msg;
  std::cout << "< $$$$$$$$$$ END HEAD $$$$$$$$$$$" << std::endl;
  _request.parseHead(msg);
  if (_request.isValid()) {
    std::cout << "request is valid" << std::endl;
    _response = Response("HTTP/1.1", "200", "OK");
    _response.setBody(
        "<html><title>200 OK</title><body>200 OK</body></html>\r\n");
    _response.setHeader("Content-Length", toString(_response.getBody().size()));
    send(_response.generate());
  } else {
    std::cout << "request is invalid" << std::endl;
    _response = Response("HTTP/1.1", "400", "Bad Request");
    _response.setBody(
        "<html><title>400 Bad Request</title><body>400 Bad "
        "Request</body></html>\r\n");
    send(_response.generate());
  }
}

void Http::OnBodyRecv(std::string msg) {
  std::cout << "$$$$$$$$$ BEGIN BODY $$$$$$$$$$ >";
  std::cout << msg;
  std::cout << "< $$$$$$$$$$ END BODY $$$$$$$$$$$" << std::endl;
  // send("Welcome! you send a BODY\n");
}

void Http::OnCgiRecv(std::string msg) {
  std::cout << "$$$$$$$$$ BEGIN CGI $$$$$$$$$$ >";
  std::cout << msg;
  std::cout << "< $$$$$$$$$$ END CGI $$$$$$$$$$$" << std::endl;
}

void Http::OnCgiTimeout()
{
  std::cout << "CGI TIMEOUT" << std::endl;
}