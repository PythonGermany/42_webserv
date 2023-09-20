#include "Http.hpp"

Http::Http(Address const &client, Address const &host) {
  this->client = client;
  this->host = host;
  this->msgdelimiter = "\r\n\r\n";
  this->msgsizelimit = 10000;
  this->msgsize = 10000;
  this->_virtualHost = NULL;
  std::stringstream ss;
  ss << "new: " << client << " for host " << host;
  Log::write(ss.str(), DEBUG);
}

Http::~Http() {
  std::stringstream ss;
  ss << "delete: " << client << " of host " << host;
  Log::write(ss.str(), DEBUG);
}

void Http::OnHeadRecv(std::string msg) {
  _request.parseHead(msg);
  File file;
  if (!_request.isValid())
    _response = processError();
  else if (_request.getVersion() != "HTTP/1.1") {
    _response = Response("HTTP/1.1", "505", "HTTP Version Not Supported");
    _response.setBody(
        "<html><title>505 HTTP Version Not Supported</title><body>505 HTTP "
        "Version Not Supported</body></html>\r\n");
    _response.setHeader("Connection", "close");
  } else {
    _response = Response("HTTP/1.1", "200", "OK");
    _virtualHost = &VirtualHost::getVirtualHosts()[0];  // TEST: remove
    Context *location = _virtualHost->matchLocation(
        _request.getUri().getPath());  // TEST: remove
    if (location != NULL) {
      Log::write("location url: " + location->getDirective("url")[0], DEBUG);
      std::string uri = location->getDirective("root")[0] +
                        _request.getUri().getPath();  // TEST: remove
      file = File(uri);
      if (_request.getUri().getPath() ==
          location->getDirective("url")[0]) {  // TEST: remove
        std::vector<std::string> locIndex = location->getDirective("index");
        for (size_t i = 0; i < locIndex.size(); i++) {
          File index(uri + locIndex[i]);
          if (index.exists() && index.readable()) {
            file = index;
            break;
          }
        }
      }
    }
    if (file.exists()) {
      if (file.readable()) {
        file.open();                     // TEST: remove
        _response.setBody(file.read());  // TEST: remove
        file.close();                    // TEST: remove
      } else {
        _response = Response("HTTP/1.1", "403", "Forbidden");
        _response.setBody(
            "<html><title>403 Forbidden</title><body>403 "
            "Forbidden</body></html>"
            "\r\n");
      }
    } else {
      _response = Response("HTTP/1.1", "404", "Not Found");
      _response.setBody(
          "<html><title>404 Not Found</title><body>404 Not "
          "Found</body></html>"
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

Response Http::processError() {
  Response response("HTTP/1.1", "400", "Bad Request");
  response.setBody(
      "<html><title>400 Bad Request</title><body>400 Bad Request</body>"
      "</html>\r\n");
  response.setHeader("Connection", "close");
  return response;
}