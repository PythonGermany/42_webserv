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
  send("HTTP/1.1 200 OK\r\n");
  send("Content-Length: 20\r\n");
  send("Content-Type: text/html\r\n");
  send("\r\n");
  send("<h1>Hello World</h1>");
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
  std::cout << "cgi timeout" << std::endl;
}