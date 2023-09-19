#include "Http.hpp"

Http::Http(Address const &client, Address const &host) {
  this->client = client;
  this->host = host;
  this->msgdelimiter = "\r\n\r\n";
  this->msgsizelimit = 10000;
  this->msgsize = 10000;
  this->_virtualHost = NULL;
  std::cout << "create: " << client << std::endl;
}

Http::~Http() { std::cout << "delete: " << client << std::endl; }

void Http::OnHeadRecv(std::string msg) {
  (void)_virtualHost;

  _request.parseHead(msg);
  if (!_request.isValid())
    processError();
  else if (_request.getVersion() != "HTTP/1.1") {
    _response = Response("HTTP/1.1", "505", "HTTP Version Not Supported");
    _response.setBody(
        "<html><title>505 HTTP Version Not Supported</title><body>505 HTTP "
        "Version Not Supported</body></html>\r\n");
    _response.setHeader("Connection", "close");
  } else {
    _response = Response("HTTP/1.1", "200", "OK");
    std::string uri = _request.getUri().getPath();  // TEST: remove
    if (uri == "/") uri = "/index.html";            // TEST: remove
    File file("website" + uri);                     // TEST: remove
    if (file.exists()) {
      file.open();                     // TEST: remove
      _response.setBody(file.read());  // TEST: remove
      file.close();                    // TEST: remove
    } else {
      _response = Response("HTTP/1.1", "404", "Not Found");
      _response.setBody(
          "<html><title>404 Not Found</title><body>404 Not Found</body></html>"
          "\r\n");
    }
  }
  _response.setHeader("Server", "webserv");
  _response.setHeader("Content-Length", toString(_response.getBody().size()));
  send(_response.generate());
  if (_request.getHeader("Connection") == "close" ||
      _response.getHeader("Connection") == "close")
    closeConnection();
}

void Http::OnBodyRecv(std::string msg) { (void)msg; }

void Http::OnCgiRecv(std::string msg) {
  std::cout << "$$$$$$$$$ BEGIN CGI $$$$$$$$$$ >";
  std::cout << msg;
  std::cout << "< $$$$$$$$$$ END CGI $$$$$$$$$$$" << std::endl;
}

void Http::OnCgiTimeout() { std::cout << "CGI TIMEOUT" << std::endl; }

void Http::processError() {
  _response = Response("HTTP/1.1", "400", "Bad Request");
  _response.setBody(
      "<html><title>400 Bad Request</title><body>400 Bad Request</body></html>"
      "\r\n");
  _response.setHeader("Connection", "close");
}