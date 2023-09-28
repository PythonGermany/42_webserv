#include "Http.hpp"

#define INDENT "\r\n                          "

Http::Http(Address const &client, Address const &host) {
  this->client = client;
  this->host = host;
  this->headDelimiter = "\r\n\r\n";
  this->headSizeLimit = 8192;
  this->_virtualHost = NULL;
  this->_context = NULL;
  this->_expectedBodySize = 0;
  this->_currBodySize = 0;
  this->_responseReady = false;
  Log::write(toString<Address &>(this->host) +
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
  _response = processPutData(_uri, msg);
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
  if (Log::getLevel() >= DEBUG &&
      _virtualHost->getContext().exists("server_name", true))
    _log += std::string(INDENT) + "VirtualHost: " +
            _virtualHost->getContext().getDirective("server_name", true)[0][0];

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
    _log += std::string(INDENT) + "Context URI: '" + contextUri + "'";

  // Check if method is allowed
  // https://datatracker.ietf.org/doc/html/rfc2616#section-10.4.6
  if (!isMethodValid()) {
    _response = processError("405", "Method Not Allowed");
    _response.setHeader("Allow", concatenate(getAllowedMethods(), ", "));
    return _response;
  }

  // Handle OPTIONS request
  // https://datatracker.ietf.org/doc/html/rfc2616#section-9.2
  if (_request.getMethod() == "OPTIONS") return processOptions();

  // Check if the request should be redirected
  if (_context->exists("redirect"))
    return processRedirect(_context->getDirective("redirect")[0][0]);

  // Process alias
  if (_context->exists("alias"))
    _uri = getContextPath("alias") + _uri.substr(contextUri.size());
  if (Log::getLevel() >= DEBUG)
    _log += std::string(INDENT) + "Resource URI: '" + _uri + "'";

  // Handle PUT request
  // https://datatracker.ietf.org/doc/html/rfc2616#section-9.6
  if (_request.getMethod() == "PUT") return processUploadHead();

  // Handle DELETE request
  // https://datatracker.ietf.org/doc/html/rfc2616#section-9.7
  if (_request.getMethod() == "DELETE") return processDelete(_uri);

  return processFile(_uri);
}

Response &Http::processFile(std::string uri) {
  File file(_context->getDirective("root", true)[0][0] + uri);

  // Add index file if needed
  if (getContextArgs() + "/" == _request.getUri().getPath() &&
      _context->exists("index")) {
    std::string path = file.getPath();
    std::vector<std::string> indexes = _context->getDirective("index", true)[0];
    for (size_t i = 0; i < indexes.size(); i++) {
      file = File(path + indexes[i]);
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

  // TODO: Handle CGI file requests

  // Load the file
  _response = Response("HTTP/1.1", "200", "OK");
  std::ifstream *body = new std::ifstream(file.getPath().c_str());
  int bodySize = file.size();
  if (body->is_open() == false || bodySize == -1) {
    delete body;
    return processError("500", "Internal Server Error");
  }
  _response.setHeader("Content-Length", toString(bodySize));
  if (_request.getMethod() != "HEAD") _response.setBody(body);
  _response.setHeader("Last-modified",
                      file.lastModified("%a, %d %b %Y %H:%M:%S"));

  // Set mime type
  std::string mimeType = VirtualHost::getMimeType(file.getExtension());
  if (!mimeType.empty()) _response.setHeader("Content-Type", mimeType);

  _responseReady = true;
  return _response;
}

Response &Http::processUploadHead() {
  if (_request.getHeader("Content-Range") != "")
    return processError("501", "Not Implemented");

  // Check if body size is specified
  std::string bodySizeStr = _request.getHeader("Content-Length");
  if (bodySizeStr.empty()) return processError("411", "Length Required");
  _expectedBodySize = fromString<size_t>(bodySizeStr);

  // Find max body size
  size_t maxBodySize = MAX_CLIENT_BODY_SIZE;
  if (_context->exists("max_client_body_size", true))
    maxBodySize = fromString<size_t>(
        _context->getDirective("max_client_body_size", true)[0][0]);

  // Check if body size is too large
  if (_expectedBodySize > maxBodySize)
    return processError("413", "Request Entity Too Large");
  _currBodySize = 0;
  bodySize = std::min((size_t)BUFFER_SIZE, _expectedBodySize);
  return _response;
}

Response &Http::processPutData(std::string uri, std::string &data) {
  // Create and write file
  if (_currBodySize == 0) {
    std::string path = _context->getDirective("root", true)[0][0] + uri;
    _newFile = !File(path).exists();

    if (!File(File(path).getDir()).exists())
      return processError("404", "Not found");

    _file.open(path.c_str(), std::ios::out | std::ios::binary);
    if (_file.is_open() == false)
      return processError("500", "Internal Server Error");
  }
  _file.write(data.c_str(), data.size());
  if (_file.good() == false)
    return processError("500", "Internal Server Error");
  _currBodySize += data.size();
  if (_currBodySize >= _expectedBodySize) {
    if (_newFile)
      _response = Response("HTTP/1.1", "201", "Created");
    else
      _response = Response("HTTP/1.1", "204", "No Content");
    _response.setHeader("Location", getAbsoluteUri(uri));
    _responseReady = true;
  } else
    bodySize = std::min((size_t)BUFFER_SIZE, _expectedBodySize - _currBodySize);
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
  if (std::remove(file.getPath().c_str()) != 0)
    return processError("500", "Internal Server Error");
  _response = Response("HTTP/1.1", "204", "No Content");
  _responseReady = true;
  return _response;
}

Response &Http::processAutoindex(std::string uri) {
  std::string path = _context->getDirective("root", true)[0][0] + uri;
  std::set<std::string> files;
  _response = Response("HTTP/1.1", "200", "OK");
  std::stringstream *body = new std::stringstream(
      "<html>\r\n<head><title>Index of " + uri + "</title></head>\r\n<body>");

  *body << "<h1>Index of " + uri + "</h1><hr><pre><a href=\"../\">../</a>\r\n";

  // Get list of files in directory
  try {
    files = File::list(path);
  } catch (const std::exception &e) {
    return processError("500", "Internal Server Error");
  }
  // Find end of longest file name to align columns
  size_t maxWidth = 50;
  std::set<std::string>::iterator itr = files.begin();
  for (; itr != files.end(); itr++) {
    if (*itr == "." && *itr == "..") continue;
    size_t size = itr->size();
    if (File(path + *itr).dir() && !endsWith(*itr, "/")) size++;
    if (size > maxWidth) maxWidth = size;
  }

  itr = files.begin();
  for (; itr != files.end(); itr++) {
    std::string file = *itr;
    if (file == "." || file == "..") continue;
    if (File(path + *itr).dir() && !endsWith(*itr, "/")) file += "/";
    *body << "<a href=\"" + percentEncode(file, "/.") + "\">" + file + "</a>";
    size_t spaceCount = maxWidth - file.size() + 5;
    for (size_t j = 0; j < spaceCount; j++) *body << " ";
    File f(path + file);
    *body << f.lastModified("%d-%m-%Y %H:%M") + "          " +
                 (f.dir() ? "-" : toString(f.size())) + "\r\n";
  }
  *body << "</pre><hr></body>\r\n</html>";
  if (_request.getMethod() != "HEAD") _response.setBody(body);
  _response.setHeader("Content-Length", toString(getStreamBufferSize(*body)));
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
  std::istream *body = NULL;
  if (_virtualHost->getContext().exists("error_page")) {
    std::vector<std::vector<std::string> > &pages =
        _virtualHost->getContext().getDirective("error_page");

    // Search for matching custom error page
    for (size_t i = 0; i < pages.size(); i++) {
      std::string pageCode = pages[i][0];
      if (pageCode != code) continue;
      std::string path =
          _context->getDirective("root", true)[0][0] + pages[i][1];
      File file(path);
      if (file.exists() && file.file() && file.readable()) {
        body = new std::ifstream(path.c_str());
        if (((std::ifstream *)body)->is_open() == false) {
          delete body;
          return processError("505", "Internal Server Error");
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
  if (body == NULL) {
    body = new std::stringstream(getDefaultBody(code, reason));
    _response.setHeader("Content-Type", "text/html");
  }
  _response.setHeader("Content-Length", toString(getStreamBufferSize(*body)));
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
  if (_response.getHeader("Content-Length") == "")
    _response.setHeader("Content-Length", toString(0));
  _response.setHeader("Server", "webserv");
  // https://datatracker.ietf.org/doc/html/rfc2616#section-14.18
  _response.setHeader("Date", getTime("%a, %d %b %Y %H:%M:%S"));

  // Check if connection should be kept alive
  if (_request.getHeader("Connection") == "keep-alive" ||
      _response.getHeader("Connection") != "close")
    _response.setHeader("Connection", "keep-alive");

  // Send response
  send(new std::istringstream(_response.generateHead()));
  if (_response.getBody() != NULL) send(_response.getBody());
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
  return false;
}

bool Http::isCgiExtension(std::string extension) const {
  if (!_context->exists("cgi", true)) return false;
  std::vector<Context> &cgis = _context->getContext("cgi", true);
  for (size_t i = 0; i < cgis.size(); i++)
    if (cgis[i].getArgs()[0] == extension) return true;
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
  std::string ret;
  if (_context->exists(token, searchTree)) {
    ret = _context->getDirective(token, searchTree)[0][0];
    if (endsWith(ret, "/")) ret = ret.substr(ret.size() - 1);
  }
  return ret;
}

std::string Http::getContextArgs() const {
  std::string ret;
  if (_context->getArgs().size() > 0) {
    ret = _context->getArgs()[0];
    if (endsWith(ret, "/")) ret = ret.substr(ret.size() - 1);
  }
  return ret;
}
