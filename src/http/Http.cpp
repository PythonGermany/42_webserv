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
  this->_error = false;
  Log::write("Http: " + toString<Address &>(this->host) +
                 " -> add: " + toString<Address &>(this->client),
             DEBUG);
}

Http::~Http() {
  Log::write(
      toString<Address &>(host) + " -> delete: " + toString<Address &>(client),
      DEBUG);
}

void Http::OnHeadRecv(std::string msg) {
  _request = Request();

  // Parse request
  _request.parseHead(msg);
  {
    Log::write(inet_ntoa(*(uint32_t *)host.addr()) + ":" +
                   toString(host.port()) + " <- " +
                   inet_ntoa(*(uint32_t *)client.addr()) + ": '" +
                   _request.getMethod() + " " + _request.getUri().getPath() +
                   " " + _request.getVersion() + "'",
               INFO);
  }
  // Find virtual host
  _virtualHost =
      VirtualHost::matchVirtualHost(host, _request.getHeader("Host"));

  // Process request
  _response = processRequest();
  if (_waitForBody == false) {
    // Set default header values
    size_t content_length = _response.getBody().size();
    if (_request.getMethod() == "HEAD") _response.setBody("");
    _response.setHeader("Server", "webserv");
    _response.setHeader("Date", getDate("%a, %d %b %Y %H:%M:%S GMT"));
    _response.setHeader("Content-Length", toString(content_length));

    // Check if connection should be kept alive
    if (_request.getHeader("Connection") == "keep-alive")
      _response.setHeader("Connection", "keep-alive");
    {
      Log::write(inet_ntoa(*(uint32_t *)host.addr()) + ":" +
                     toString(host.port()) + " -> " +
                     inet_ntoa(*(uint32_t *)client.addr()) + ": '" +
                     _response.getVersion() + " " + _response.getStatus() +
                     " " + _response.getReason() + "'",
                 INFO);
    }

    // Send response
    send(_response.generate());

    // Close connection if needed or asked for
    if (_response.getHeader("Connection") == "close" ||
        _request.getHeader("Connection") == "close")
      closeConnection();
  }
}

void Http::OnBodyRecv(std::string msg) {
  if (_waitForBody == false) return;
  _request.setBody(msg);
  _response = processUploadBody(_request.getUri().getPath());

  // Set default header values
  size_t content_length = _response.getBody().size();
  _response.setHeader("Server", "webserv");
  _response.setHeader("Date", getDate("%a, %d %b %Y %H:%M:%S GMT"));
  _response.setHeader("Content-Length", toString(content_length));

  // Check if connection should be kept alive
  if (_request.getHeader("Connection") == "keep-alive")
    _response.setHeader("Connection", "keep-alive");
  {
    Log::write(inet_ntoa(*(uint32_t *)client.addr()) + " <- " +
                   _response.getVersion() + " " + _response.getStatus() + " " +
                   _response.getReason(),
               INFO);
  }

  // Send response
  send(_response.generate());

  // Close connection if needed or asked for
  if (_response.getHeader("Connection") == "close" ||
      _request.getHeader("Connection") == "close")
    closeConnection();

  msgsize = 10000;       // TODO: bad implementation ?
  _waitForBody = false;  // TODO: bad implementation ?
}

void Http::OnCgiRecv(std::string msg) {
  std::cout << "$$$$$$$$$ BEGIN CGI $$$$$$$$$$ >";
  std::cout << msg;
  std::cout << "< $$$$$$$$$$ END CGI $$$$$$$$$$$" << std::endl;
}

void Http::OnCgiTimeout() { std::cout << "CGI TIMEOUT" << std::endl; }

Response &Http::processRequest() {
  if (_virtualHost == NULL) return processError("500", "Internal Server Error");
  {
    Log::write(
        "VirtualHost: " + toString<Address &>(_virtualHost->getAddress()),
        DEBUG);
  }

  // Check if the request is valid
  if (_request.isValid() == false) return processError("400", "Bad Request");

  if (_request.getVersion() != "HTTP/1.1")
    return processError("505", "HTTP Version Not Supported");

  // Find correct location context
  _context = _virtualHost->matchLocation(_request.getUri().getPath());
  if (_context == NULL) return processError("500", "Internal Server Error");
  {
    std::string path = "/";
    if (_context->argCount() > 0) path = _context->getArgs()[0];
    Log::write("Location: " + path, DEBUG);
  }

  // Check if method is allowed
  if (isMethodValid(_context, _request) == false) return _response;

  std::string locUri = "";
  if (_context->getName() == "location") locUri = _context->getArgs()[0];
  std::string uri = _request.getUri().getPath();
  uri = uri.substr(locUri.size());

  // Handle PUT request
  if (_request.getMethod() == "PUT") return processUploadHead();

  // Handle DELETE request
  if (_request.getMethod() == "DELETE") return processDelete(uri);

  // Check if the request should be redirected
  if (_context->exists("redirect", true))
    return processRedirect(_context->getDirective("redirect", true)[0]);

  // Process the request as a file
  std::string index = _context->getDirective("index", true)[0];
  if (uri == "/") uri += index;
  return processFile(uri);
}

Response &Http::processFile(std::string uri) {
  std::string path = _context->getDirective("root", true)[0] + uri;
  File file(path);

  if (!file.exists()) return processError("404", "Not Found");
  if (file.dir()) {
    if (!endsWith(path, "/")) return processRedirect(uri + "/");
    if (_context->exists("autoindex", true) &&
        _context->getDirective("autoindex", true)[0] == "on")
      return processAutoindex(uri);
    return processError("403", "Forbidden");
  } else if (!file.readable())
    return processError("403", "Forbidden");

  // Load the file
  _response = Response("HTTP/1.1", "200", "OK");
  try {
    file.open(O_RDONLY);
    _response.setBody(file.read());
    file.close();
  } catch (const std::exception &e) {
    return processError("500", "Internal Server Error");
  }

  // Set mime type
  std::string mimeType = VirtualHost::getMimeType(file.getExtension());
  if (!mimeType.empty()) _response.setHeader("Content-Type", mimeType);

  return _response;
}

Response &Http::processUploadHead() {
  // Check if server is configured for upload
  if (_context->exists("upload", true) == false)
    return processError("503", "Service Unavailable");

  // Check if body size is specified
  std::string bodySize = _request.getHeader("Content-Length");
  if (bodySize.empty()) return processError("411", "Length Required");
  msgsize = fromString<size_t>(bodySize);

  // Find max body size
  size_t maxBodySize = MAX_CLIENT_BODY_SIZE;
  if (_context->exists("max_client_body_size", true))
    maxBodySize = fromString<size_t>(
        _context->getDirective("max_client_body_size", true)[0]);

  // Check if body size is too large
  if (msgsize > maxBodySize)
    return processError("413", "Request Entity Too Large");

  _waitForBody = true;
  return _response;
}

Response &Http::processUploadBody(std::string uri) {
  if (_context->exists("upload") == false)
    return processError("500", "Internal Server Error");

  std::string root = _context->getDirective("root", true)[0];
  std::string upload = _context->getDirective(
      "upload", true)[0];  // TODO: Decide if upload path should be relative to
                           // root or to location path
  std::string locPath = "";
  if (_context->getName() == "location") locPath = _context->getArgs()[0];
  uri = upload + uri.substr(locPath.size());

  // Create and write file
  bool newFile = true;
  File file(root + uri);
  try {
    if (!file.exists())
      file.create();
    else
      newFile = false;
    file.open(O_WRONLY | O_TRUNC);
    file.write(_request.getBody());
    file.close();
  } catch (const std::exception &e) {
    return processError("500", "Internal Server Error");
  }
  if (newFile) {
    _response = Response("HTTP/1.1", "201", "Created");
    _response.setBody(_request.getBody());
  } else
    _response = Response("HTTP/1.1", "204", "No Content");
  _response.setHeader("Location", getAbsoluteUri(locPath + uri));
  return _response;
}

Response &Http::processDelete(std::string uri) {
  std::string path = _context->getDirective("root", true)[0] + uri;
  File file(path);

  if (!file.exists()) return processError("404", "Not Found");
  if (file.dir()) return processError("403", "Forbidden");
  if (!file.readable()) return processError("403", "Forbidden");
  try {
    file.remove();
  } catch (const std::exception &e) {
    return processError("500", "Internal Server Error");
  }
  _response = Response("HTTP/1.1", "204", "No Content");
  return _response;
}

Response &Http::processAutoindex(std::string uri) {
  std::string path = _context->getDirective("root", true)[0] + uri;
  _response = Response("HTTP/1.1", "200", "OK");
  std::string body = "<html><title>Index of " + uri + "</title><body>";
  body += "<h1>Index of " + uri + "</h1><hr><pre>";
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

Response &Http::processRedirect(std::string uri) {
  _response = Response("HTTP/1.1", "301", "Moved Permanently");
  _response.setHeader("Location", getAbsoluteUri(uri));
  return _response;
}

Response &Http::processError(std::string code, std::string reason) {
  _response = Response("HTTP/1.1", code, reason);
  std::string body;
  if (_virtualHost != NULL) {
    std::vector<Context> &errors =
        _virtualHost->getContext().getContext("error_page");

    // Search for matching custom error page
    for (size_t i = 0; i < errors.size(); i++) {
      std::string error = errors[i].getArgs()[0];
      if (error == code) {
        std::string path = _context->getDirective("root", true)[0] +
                           errors[i].getDirective("path")[0];
        File file(path);
        if (file.exists()) {
          if (file.file() && file.readable()) {
            try {
              file.open(O_RDONLY);
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
          _response = Response("HTTP/1.1", code, reason);
        }
        break;
      }
    }
  }

  // If no custom error page was found, use default
  if (body.empty()) {
    body = getDefaultBody(code, reason);
    _response.setHeader("Content-Type", "text/html");
  }

  _response.setBody(body);
  _error = true;
  return _response;
}

std::string Http::getDefaultBody(std::string code, std::string reason) {
  return "<html>\r\n<head><title>" + code + " " + reason +
         "</title></head>\r\n<body>\r\n<center><h1>" + code + " " + reason +
         "</h1></center>\r\n<hr><center>webserv</center>\r\n</body>\r\n</"
         "html>\r\n";
}

std::string Http::getFieldValue(std::vector<std::string> const &values) {
  std::string value;
  for (size_t i = 0; i < values.size(); i++) {
    if (i != 0) value += ", ";
    value += values[i];
  }
  return value;
}

std::string Http::getAbsoluteUri(std::string uri) {
  Uri ret(uri);
  if (ret.getHost().empty()) {
    std::string host = _request.getHeader("Host");
    size_t pos = host.find(":");
    if (pos != std::string::npos) {
      ret.setHost(host.substr(0, pos));
      ret.setPort(host.substr(pos + 1));
    } else
      ret.setHost(host);
  }
  return ret.generate();
}

bool Http::isMethodValid(Context *context, Request &request) {
  if (context->exists("allow", true)) {
    std::vector<std::string> methods = context->getDirective("allow", true);
    if (std::find(methods.begin(), methods.end(), request.getMethod()) !=
        methods.end())
      return true;
    _response = processError("405", "Method Not Allowed");
    return false;
  }
  return true;
}