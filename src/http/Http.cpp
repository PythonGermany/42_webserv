#include "Http.hpp"

Cache Http::_cache;

#define INDENT "\r\n                          "

Http::Http(Address const &client, Address const &host) {
  this->client = client;
  this->host = host;
  this->headDelimiter = "\r\n\r\n";
  this->headSizeLimit = 8192;
  this->_virtualHost = NULL;
  this->_context = NULL;
  this->_responseReady = false;
  this->_writeBufferPos = 0;  // TODO: Pythongermany code
  Log::write(toString<Address &>(this->host) +
                 " -> add: " + toString<Address &>(this->client),
             DEBUG);
}

Http::~Http() {
  Log::write(
      toString<Address &>(host) + " -> delete: " + toString<Address &>(client),
      DEBUG);
}

bool Http::updateCache() { return _cache.update(); }

const Cache &Http::getCache() { return _cache; }

void Http::OnHeadRecv(std::string msg) {
  _request = Request();
  _writeBufferPos = 0;  // TODO: Pythongermany code

  // Parse request
  _request.parseHead(msg);
  _log = toString<Address &>(client) + ": '" + _request.getMethod() + " " +
         _request.getUri().generate() + " " + _request.getVersion() + "' -> " +
         toString<Address &>(host);

  // If possible use host from absolute uri otherwise use host from header
  // https://datatracker.ietf.org/doc/html/rfc2616#section-5.2
  std::string requestHost = _request.getHeader("Host");
  if (!_request.getUri().getHost().empty()) {
    requestHost = _request.getUri().getHost();
    if (!_request.getUri().getPort().empty())
      requestHost += ":" + _request.getUri().getPort();
  }

  // Find virtual host
  _virtualHost = VirtualHost::matchVirtualHost(host, requestHost);

  // Process request
  _response = processRequest();
  if (_responseReady) sendResponse();
}

void Http::OnBodyRecv(std::string msg) {
  _request.setBody(msg);
  _response = processUploadBody(_uri);
  if (_responseReady) sendResponse();
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
        "VirtualHost: " + _virtualHost->getAddress(),
        DEBUG);
  }

  // Check if the request is valid
  // https://datatracker.ietf.org/doc/html/rfc2616#section-10.4.1
  if (_request.getUri().decode() || !_request.isValid())
    return processError("400", "Bad Request");

  if (_request.getVersion() != HTTP_VERSION)
    return processError("505", "HTTP Version Not Supported");

  // Check if method is implemented
  // https://datatracker.ietf.org/doc/html/rfc2616#section-10.5.2
  if (!isMehodImplemented(_request.getMethod()))
    return processError("501", "Not Implemented");

  // Find correct location context
  _uri = _request.getUri().getPath();
  _context = _virtualHost->matchLocation(_uri);
  if (_context == NULL) return processError("500", "Internal Server Error");

  std::string contextUri = getContextArgs();
  if (Log::getLevel() >= DEBUG)
    _log += std::string(INDENT) +
            "Context URI: " + (contextUri != "" ? contextUri : "/");

  // Check if method is allowed
  // https://datatracker.ietf.org/doc/html/rfc2616#section-10.4.6
  if (!isMethodValid()) {
    _responseReady = true;
    return _response;
  }

  // Handle OPTIONS request
  // https://datatracker.ietf.org/doc/html/rfc2616#section-9.2
  if (_request.getMethod() == "OPTIONS") return processOptions();

  // Process alias
  if (_context->exists("alias"))
    _uri = getContextPath("alias") + _uri.substr(contextUri.size());
  if (Log::getLevel() >= DEBUG)
    _log += std::string(INDENT) + "Resource URI: " + _uri;

  // Handle PUT request
  // https://datatracker.ietf.org/doc/html/rfc2616#section-9.6
  if (_request.getMethod() == "PUT") return processUploadHead();

  // Handle DELETE request
  // https://datatracker.ietf.org/doc/html/rfc2616#section-9.7
  if (_request.getMethod() == "DELETE") return processDelete(_uri);

  // Check if the request should be redirected
  if (_context->exists("redirect"))
    return processRedirect(_context->getDirective("redirect")[0][0]);

  return processFile(_uri);
}

Response &Http::processFile(std::string uri) {
  std::string path = _context->getDirective("root", true)[0][0] + uri;
  File file(path);

  // Add index file if needed
  if (endsWith(uri, "/") && _context->exists("index", true)) {
    std::vector<std::string> indexes = _context->getDirective("index", true)[0];
    std::string index;
    for (size_t i = 0; i < indexes.size(); i++) {
      index = indexes[i];
      if (index[0] == '/') index = index.substr(1);
      file = File(path + index);
      if (file.exists() && file.file() && file.readable()) break;
    }
  }

  if (!file.exists()) {
    // Redirect to directory if file does not exist
    if (!endsWith(file.getPath(), "/") && file.getExtension() == "")
      return processRedirect(uri + "/");
    return processError("404", "Not Found");
  }
  if (file.dir()) {
    if (!endsWith(file.getPath(), "/")) return processRedirect(uri + "/");
    if (_context->exists("autoindex", true) &&
        _context->getDirective("autoindex", true)[0][0] == "on")
      return processAutoindex(uri);
    return processError("403", "Forbidden");
  } else if (!file.readable())
    return processError("403", "Forbidden");

  // Load the file
  _response = Response("HTTP/1.1", "200", "OK");
  try {
    std::string body;
    bool isCached = _cache.isCached(path);
    if (!_cache.isCached(path) || _cache.isStale(path, std::time(NULL))) {
      file.open(O_RDONLY);
      body = file.read();
      isCached ? _cache.update(path, body) : _cache.add(path, body);
    } else
      body = _cache.get(path);
    if (Log::getLevel() >= DEBUG)
      _log += std::string(INDENT) + "Cache : " + _cache.info();
    if (_request.getMethod() != "HEAD") _response.setBody(body);
    _response.setHeader("Content-Length", toString(body.size()));
    file.close();
  } catch (const std::exception &e) {
    return processError("500", "Internal Server Error");
  }
  _response.setHeader("Last-modified",
                      file.lastModified("%a, %d %b %Y %H:%M:%S"));

  // Set mime type
  std::string mimeType = VirtualHost::getMimeType(file.getExtension());
  if (!mimeType.empty()) _response.setHeader("Content-Type", mimeType);

  _responseReady = true;
  return _response;
}

Response &Http::processUploadHead() {
  // Check if body size is specified
  std::string bodySizeStr = _request.getHeader("Content-Length");
  if (bodySizeStr.empty()) return processError("411", "Length Required");
  bodySize = fromString<size_t>(bodySizeStr);

  // Find max body size
  size_t maxBodySize = MAX_CLIENT_BODY_SIZE;
  if (_context->exists("max_client_body_size", true))
    maxBodySize = fromString<size_t>(
        _context->getDirective("max_client_body_size", true)[0][0]);

  // Check if body size is too large
  if (bodySize > maxBodySize)
    return processError("413", "Request Entity Too Large");
  return _response;
}

Response &Http::processUploadBody(std::string uri) {
  std::string path = _context->getDirective("root", true)[0][0] + uri;

  // Create and write file
  File file(path);
  bool newFile = !file.exists();
  try {
    if (!file.exists()) file.create();
    file.open(O_WRONLY | O_TRUNC);
    file.write(_request.getBody());
    file.close();
  } catch (const std::exception &e) {
    return processError("500", "Internal Server Error");
  }
  if (newFile)
    _response = Response("HTTP/1.1", "201", "Created");
  else
    _response = Response("HTTP/1.1", "204", "No Content");
  _response.setHeader("Location", getAbsoluteUri(uri));
  _responseReady = true;
  return _response;
}

Response &Http::processOptions() {
  _response = Response("HTTP/1.1", "200", "OK");
  if (_uri == "/*") {
    _response.setHeader("Allow", concatenate(getAllowedMethods(false), ", "));
  } else
    _response.setHeader("Allow", concatenate(getAllowedMethods(), ", "));
  _responseReady = true;
  return _response;
}

Response &Http::processDelete(std::string uri) {
  std::string path = _context->getDirective("root", true)[0][0] + uri;
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
  _responseReady = true;
  return _response;
}

Response &Http::processAutoindex(std::string uri) {
  std::string path = _context->getDirective("root", true)[0][0] + uri;
  _response = Response("HTTP/1.1", "200", "OK");
  std::string body =
      "<html>\r\n<head><title>Index of " + uri + "</title></head>\r\n<body>";
  std::vector<std::string> files;

  body += "<h1>Index of " + uri + "</h1><hr><pre><a href=\"../\">../</a>\r\n";

  // Get list of files in directory
  try {
    files = File::list(path);
  } catch (const std::exception &e) {
    return processError("500", "Internal Server Error");
  }
  // Find end of longest file name to align columns
  size_t maxWidth = 50;
  for (size_t i = 0; i < files.size(); i++) {
    if (files[i] == "." && files[i] == "..") continue;
    if (File(path + files[i]).dir() && !endsWith(files[i], "/"))
      files[i] += "/";
    if (files[i].size() > maxWidth) maxWidth = files[i].size();
  }

  for (size_t i = 0; i < files.size(); i++) {
    std::string file = files[i];
    if (file == "./" || file == "../") continue;
    body +=
        "<a href=\"" + percentEncode(files[i], "/.") + "\">" + file + "</a>";
    size_t spaceCount = maxWidth - file.size() + 5;
    for (size_t j = 0; j < spaceCount; j++) body += " ";
    File f(path + files[i]);
    body += f.lastModified("%d-%m-%Y %H:%M") + "          " +
            (f.dir() ? "-" : toString(f.size())) + "\r\n";
  }
  body += "</pre><hr></body>\r\n</html>\r\n";
  if (_request.getMethod() != "HEAD") _response.setBody(body);
  _response.setHeader("Content-Length", toString(body.size()));
  _responseReady = true;
  return _response;
}

Response &Http::processRedirect(std::string uri) {
  _response = Response("HTTP/1.1", "301", "Moved Permanently");
  _response.setHeader("Location", getAbsoluteUri(uri));
  _responseReady = true;
  return _response;
}

Response &Http::processError(std::string code, std::string reason) {
  _response = Response("HTTP/1.1", code, reason);
  std::string body;
  if (_virtualHost->getContext().exists("error_page", true)) {
    std::vector<std::vector<std::string> > &pages =
        _virtualHost->getContext().getDirective("error_page", true);

    // Search for matching custom error page
    for (size_t i = 0; i < pages.size(); i++) {
      std::string pageCode = pages[i][0];
      if (pageCode != code) continue;
      std::string path =
          _context->getDirective("root", true)[0][0] + pages[i][1];
      File file(path);
      if (file.exists() && file.file() && file.readable()) {
        try {
          file.open(O_RDONLY);
          body = file.read();
          file.close();
        } catch (const std::exception &e) {
          return processError("500", "Internal Server Error");
        }
        // Set mime type
        std::string mimeType = VirtualHost::getMimeType(file.getExtension());
        if (!mimeType.empty()) _response.setHeader("Content-Type", mimeType);
      } else if (code != "404")
        return processError("404", "Not Found");
      break;
    }
  }

  // If no custom error page was found, use default
  if (body.empty()) {
    body = getDefaultBody(code, reason);
    _response.setHeader("Content-Type", "text/html");
  }
  _response.setHeader("Content-Length", toString(body.size()));
  if (_request.getMethod() != "HEAD") _response.setBody(body);
  _responseReady = true;
  return _response;
}

std::string Http::getDefaultBody(std::string code, std::string reason) const {
  return "<html>\r\n<head><title>" + code + " " + reason +
         "</title></head>\r\n<body>\r\n<center><h1>" + code + " " + reason +
         "</h1></center>\r\n<hr><center>webserv</center>\r\n</body>\r\n</"
         "html>\r\n";
}

void Http::sendResponse() {
  if (_responseReady == false) {
    Log::write("WARNING: Trying to send response before it is ready", WARNING,
               BRIGHT_YELLOW);
    return;
  }

  // Set default header values
  _response.setHeader("Server", "webserv");
  // https://datatracker.ietf.org/doc/html/rfc2616#section-14.18
  _response.setHeader("Date", getTime("%a, %d %b %Y %H:%M:%S"));
  if (_response.getHeader("Content-Length").empty())
    _response.setHeader("Content-Length", toString(_response.getBody().size()));

  // Check if connection should be kept alive
  if (_request.getHeader("Connection") == "keep-alive" ||
      _response.getHeader("Connection") != "close")
    _response.setHeader("Connection", "keep-alive");

  // Send response
  send(_response.generate());
  _responseReady = false;
  bodySize = WAIT_FOR_HEAD;

  // Close connection if needed or asked for // TODO: Ask if it is guaranteed
  // that everythign is sent before closing
  if (_response.getHeader("Connection") == "close" ||
      _request.getHeader("Connection") == "close")
    closeConnection();

  Log::write(_log + std::string(INDENT) + "'" + _response.getVersion() + " " +
                 _response.getStatus() + " " + _response.getReason() + "' " +
                 _request.getHeader("User-Agent"),
             INFO);
}

std::string Http::getAbsoluteUri(std::string uri) const {
  Uri ret(uri);
  if (ret.getScheme().empty()) ret.setScheme("http");
  if (ret.getHost().empty()) {
    std::string host = _request.getHeader("Host");
    size_t pos = host.find(":");
    if (pos != std::string::npos) {
      ret.setHost(host.substr(0, pos));
      ret.setPort(host.substr(pos + 1));
    } else
      ret.setHost(host);
  }
  return ret.encode();
}

bool Http::isMehodImplemented(std::string method) const {
  std::string methods[HTTP_METHOD_COUNT] = HTTP_METHODS;
  for (size_t i = 0; i < HTTP_METHOD_COUNT; i++)
    if (methods[i] == method) return true;
  return false;
}

bool Http::isMethodValid() {
  std::vector<std::string> allowedMethods = getAllowedMethods();
  for (size_t i = 0; i < allowedMethods.size(); i++)
    if (allowedMethods[i] == _request.getMethod()) return true;
  _response = processError("405", "Method Not Allowed");
  if (_response.getStatus() == "405")
    _response.setHeader("Allow", concatenate(allowedMethods, ", "));
  _responseReady = true;
  return false;
}

std::vector<std::string> Http::getAllowedMethods(bool forUri) const {
  std::vector<std::string> ret;
  if (forUri == false) {
    std::string methods[HTTP_METHOD_COUNT] = HTTP_METHODS;
    for (size_t i = 0; i < HTTP_METHOD_COUNT; i++) ret.push_back(methods[i]);
  } else if (_context->exists("allow", true))
    ret = _context->getDirective("allow", true)[0];
  else {
    std::string methods[HTTP_DEFAULT_METHOD_COUNT] = HTTP_DEFAULT_METHODS;
    for (size_t i = 0; i < HTTP_DEFAULT_METHOD_COUNT; i++)
      ret.push_back(methods[i]);
  }
  return ret;
}

std::string Http::getContextPath(std::string token, bool searchTree) const {
  if (_context->exists(token, searchTree) &&
      _context->getDirective(token)[0][0] != "/")
    return _context->getDirective(token, searchTree)[0][0];
  return "";
}

std::string Http::getContextArgs() const {
  if (_context->getArgs().size() > 0) return _context->getArgs()[0];
  return "";
}