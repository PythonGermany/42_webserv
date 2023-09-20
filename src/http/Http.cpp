#include "Http.hpp"

Http::Http(Address const &client, Address const &host) {
  this->client = client;
  this->host = host;
  this->msgdelimiter = "\r\n\r\n";
  this->msgsizelimit = 10000;
  this->msgsize = 10000;
  this->_virtualHost = NULL;
  this->_context = NULL;
  this->_waitForBody = false;
  std::stringstream ss;
  ss << "connect: " << client << " -> " << host;
  Log::write(ss.str(), DEBUG);
}

Http::~Http() {
  std::stringstream ss;
  ss << "timeout: " << client << " <- " << host;
  Log::write(ss.str(), DEBUG);
}

void Http::OnHeadRecv(std::string msg) {
  _request = Request();
  int parseHead = _request.parseHead(msg);
  Log::write(inet_ntoa(*(uint32_t *)client.addr()) + " -> " +
                 _request.getMethod() + " " + _request.getUri().getPath() +
                 " " + _request.getVersion(),
             INFO);
  if (parseHead != 0 || _request.isValid() == false) {
    _virtualHost = NULL;  // TODO: Find the correct default virtual host
    _response = processError("400", "Bad Request");
  } else {
    _virtualHost = NULL;  // TODO: Find the correct virtual host
    _virtualHost = &VirtualHost::getVirtualHosts()[0];
    _response = processRequest();
  }
  size_t content_length = _response.getBody().size();
  if (_request.getMethod() == "HEAD") _response.setBody("");
  if (_waitForBody == false) {
    _response.setHeader("Server", "webserv");
    _response.setHeader("Date", getDate("%a, %d %b %Y %H:%M:%S GMT"));
    _response.setHeader("Content-Length", toString(content_length));
    if (_request.getHeader("Connection") == "keep-alive")
      _response.setHeader("Connection", "keep-alive");
    Log::write(inet_ntoa(*(uint32_t *)client.addr()) + " <- " +
                   _response.getVersion() + " " + _response.getStatus() + " " +
                   _response.getReason(),
               INFO);
    send(_response.generate());
    if (_response.getHeader("Connection") == "close" ||
        _request.getHeader("Connection") == "close")
      closeConnection();
  }
}

void Http::OnBodyRecv(std::string msg) {
  std::cout << "$$$$$$$$$ BEGIN BODY $$$$$$$$$$ >";
  std::cout << msg;
  std::cout << "< $$$$$$$$$$ END BODY $$$$$$$$$$$" << std::endl;
}

void Http::OnCgiRecv(std::string msg) {
  std::cout << "$$$$$$$$$ BEGIN CGI $$$$$$$$$$ >";
  std::cout << msg;
  std::cout << "< $$$$$$$$$$ END CGI $$$$$$$$$$$" << std::endl;
}

void Http::OnCgiTimeout() { std::cout << "CGI TIMEOUT" << std::endl; }

Response &Http::processRequest() {
  if (_virtualHost == NULL) return processError("500", "Internal Server Error");
  if (_request.getVersion() != "HTTP/1.1")
    return processError("505", "HTTP Version Not Supported");

  // Find correct location context
  _context = _virtualHost->matchLocation(_request.getUri().getPath());
  if (_context == NULL) return processError("404", "Not Found");
  Log::write("location: " + _context->getDirective("url")[0], DEBUG);

  // Check if method is allowed
  bool methodAllowed = false;
  std::vector<std::string> &methods = _context->getDirective("method");
  for (size_t i = 0; i < methods.size(); i++) {
    if (methods[i] == _request.getMethod()) {
      methodAllowed = true;
      break;
    }
  }
  if (methodAllowed == false) {
    processError("405", "Method Not Allowed");
    std::string allowed;
    for (size_t i = 0; i < methods.size(); i++) {
      if (i != 0) allowed += ", ";
      allowed += methods[i];
    }
    _response.setHeader("Allow", allowed);
    return _response;
  }

  // Check if the server should wait for the body
  if (_request.getMethod() == "POST") {
    std::string bodySize = _request.getHeader("Content-Length");
    if (bodySize.empty()) return processError("411", "Length Required");
    _waitForBody = true;
    std::stringstream ss(bodySize);
    ss >> msgsize;
    return _response;
  }

  // Check if the request should be redirected
  if (_context->exists("redirect"))
    return processRedirect(_context->getDirective("redirect")[0]);

  // Process the request as a file
  std::string root = _context->getDirective("root")[0];
  std::string index = _context->getDirective("index")[0];
  std::string uri = _request.getUri().getPath();
  std::string path = root + uri;
  if (uri == _context->getDirective("url")[0] && endsWith(uri, "/"))
    path += index;
  return processFile(path);
}

Response &Http::processFile(std::string path) {
  File file(path);
  if (!file.exists())
    return processError("404", "Not Found");
  else if (file.dir()) {
    if (!endsWith(path, "/"))
      return processRedirect(_request.getUri().getPath() + "/");
    if (_context->exists("autoindex") &&
        _context->getDirective("autoindex")[0] == "on")
      return processAutoindex(path);
    return processError("403", "Forbidden");
  } else if (!file.readable())
    return processError("403", "Forbidden");

  // Load the file
  _response = Response("HTTP/1.1", "200", "OK");
  try {
    file.open();
    _response.setBody(file.read());
    file.close();
  } catch (const std::exception &e) {
    return processError("500", "Internal Server Error");
  }
  std::string mimeType = VirtualHost::getMimeType(file.getExtension());
  if (!mimeType.empty()) _response.setHeader("Content-Type", mimeType);
  return _response;
}

Response &Http::processAutoindex(std::string path) {
  _response = Response("HTTP/1.1", "200", "OK");
  std::string body = "<html><title>Index of " + path + "</title><body>";
  body += "<h1>Index of " + path + "</h1><hr><pre>";
  std::vector<std::string> files;
  try {
    files = File::list(path);
  } catch (const std::exception &e) {
    return processError("500", "Internal Server Error");
  }
  for (size_t i = 0; i < files.size(); i++) {
    std::string file = files[i];
    if (file == "." || file == "..") continue;
    std::string href = _request.getUri().getPath() + file;
    body += "<a href=\"" + href + "\">" + file + "</a>";
    for (size_t j = 0; j < 50 - file.size(); j++) body += " ";
    body += "|" + toString(File(path + files[i]).size());
    body += "<br>";
  }
  body += "</pre><hr></body></html>\r\n";
  _response.setBody(body);
  _response.setHeader("Connection", "close");
  return _response;
}

Response &Http::processRedirect(std::string path) {
  _response = Response("HTTP/1.1", "301", "Moved Permanently");
  std::string body =
      "<html><title>301 Moved Permanently</title><body>301 "
      "Moved Permanently</body></html>\r\n";
  _response.setBody(body);
  _response.setHeader("Location", path);
  return _response;
}

Response &Http::processError(std::string code, std::string reason) {
  _response = Response("HTTP/1.1", code, reason);
  std::string body;
  if (_virtualHost != NULL) {
    if (_context == NULL) return processError("404", "Not Found");

    std::vector<Context> &errors =
        _virtualHost->getContext().getContext("error_page");
    for (size_t i = 0; i < errors.size(); i++) {
      std::string error = errors[i].getDirective("code")[0];
      if (error == code) {
        std::string path = _context->getDirective("root")[0] +
                           errors[i].getDirective("path")[0];
        File file(path);
        if (file.exists()) {
          if (file.file() && file.readable()) {
            try {
              file.open();
              body = file.read();
              file.close();
            } catch (const std::exception &e) {
              processError("500", "Internal Server Error");
            }
          }
        } else if (code != "404")
          processError("404", "Not Found");
        else {
          code = "404";
          reason = "Not Found";
        }
      }
    }
  }
  if (body.empty()) body = getDefaultBody(code, reason);
  _response.setBody(body);
  return _response;
}

std::string Http::getDefaultBody(std::string code, std::string reason) {
  return "<html>\r\n<head><title>" + code + " " + reason +
         "</title></head>\r\n<body>\r\n<center><h1>" + code + " " + reason +
         "</h1></center>\r\n<hr><center>webserv</center>\r\n</body>\r\n</"
         "html>\r\n";
}