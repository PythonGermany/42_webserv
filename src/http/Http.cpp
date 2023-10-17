#include "Http.hpp"

Http::Http(Address const &client, Address const &host)
    : AConnection(host, client) {
  setReadState(STATUS);
  this->headSizeLimit = 8192;
  this->_virtualHost = NULL;
  this->_context = NULL;
  this->_expectedBodySize = 0;
  this->_currBodySize = 0;
  accessLog_g.write(toString<Address &>(this->host) +
                        " -> add: " + toString<Address &>(this->client),
                    DEBUG);
}

Http::~Http() {
  accessLog_g.write(
      toString<Address &>(host) + " -> delete: " + toString<Address &>(client),
      DEBUG);
}

void Http::OnStatusRecv(std::string msg) {
  accessLog_g.write("HTTP status: '" + msg + "'", VERBOSE);

  if (startsWith(msg, readDelimiter)) return;
  _request = Request();

  // TODO: Implement uri dot resolving somewhere here or maybe in the uri
  // class
  // -> uri class prob better
  msg.erase(msg.size() - 2, 2);
  bool parseRet = _request.parseStatus(msg) || _request.getUri().decode() ||
                  _request.getUri().pathOutOfBound();

  _log = toString<Address &>(client) + ": " + _request.getMethod() + " " +
         _request.getUri().generate() + " " + _request.getVersion() + " -> " +
         toString<Address &>(host);

  if (parseRet)
    processError("400", "Bad Request");
  else if (isHttpVersionValid(_request.getVersion()) == false) {
    processError("505", "HTTP Version Not Supported");
    _response.setHeader("Upgrade", "HTTP/" HTTP_VERSION);
    _response.setHeader("Connection", "Upgrade");
  } else if (isMehodImplemented(_request.getMethod()) == false)
    processError("501", "Not Implemented");
  if (_response.isReady()) return sendResponse();
  setReadState(HEAD);
}

void Http::OnHeadRecv(std::string msg) {
  accessLog_g.write("HTTP head: '" + msg + "'", VERBOSE);

  // Parse request
  if (_request.parseHeaderFields(msg)) {
    processError("400", "Bad Request");
    sendResponse();
  }

  // If possible use host of absolute uri otherwise use host of header
  std::string requestHost = _request.getHeader("Host");
  if (_request.getUri().getHost().size() > 0) {
    requestHost = _request.getUri().getHost();
    std::string port = _request.getUri().getPort();
    if (port.size() > 0 && port != "80")
      requestHost += ":" + _request.getUri().getPort();
  }

  if (_request.getHeader("Host").size() > 0) {
    _virtualHost = VirtualHost::matchVirtualHost(host, requestHost);
    processRequest();
  } else
    processError("400", "Bad Request");

  if (_response.isReady()) sendResponse();
}

void Http::OnChunkSizeRecv(std::string msg) {
  accessLog_g.write("HTTP chunk size: '" + msg + "'", VERBOSE);
  size_t end = msg.find(';');
  if (end == std::string::npos) end = msg.size() - readDelimiter.size();

  bodySize = 0;
  if (std::sscanf(msg.substr(0, end).c_str(), "%lx", &bodySize) == EOF) {
    errorLog_g.write("OnChunkSizeRecv(): sscanf failure", DEBUG, BRIGHT_RED);
    processError("500", "Internal server error", true);
  } else if (isBodySizeValid(_currBodySize + bodySize) == false)
    processError("413", "Request Entity Too Large", true);
  if (_response.isReady()) return sendResponse();

  bodySize += 2;
  setReadState(BODY);
}

void Http::OnBodyRecv(std::string msg) {
  accessLog_g.write("HTTP body: '" + msg + "'", VERBOSE);
  if (_request.getHeader("Transfer-Encoding") == "chunked") {
    if (!endsWith(msg, "\r\n")) return processError("400", "Bad Request", true);
    msg.erase(msg.size() - 2, 2);
  }
  if (_request.getMethod() == "PUT")
    processPutData(msg);
  else if (_request.getMethod() == "POST")
    processPostData(msg);
  if (_response.isReady()) sendResponse();
}

void Http::OnCgiRecv(std::string msg) {
  accessLog_g.write("CGI out: \"" + msg + "\"", VERBOSE);
  _response.init("HTTP/1.1", "200", "OK");
  int bodySize = msg.size();
  _response.setBody(new std::istringstream(msg));

  std::string line;
  while (std::getline(*_response.getBody(), line)) {
    bodySize -= line.size() + 1;
    if (line.empty() == false && line[line.size() - 1] == '\r')
      line.erase(line.size() - 1);
    if (line.empty()) break;
    std::string name = line.substr(0, line.find(':'));
    if (line.find(": ") == name.size())
      line.erase(0, name.size() + 2);
    else {
      errorLog_g.write("ERROR: invalid format in cgi response header: " + line,
                       ERROR);
      return OnCgiError();
    }
    for (std::string::iterator it = name.begin(); it != name.end(); ++it)
      *it = std::tolower(*it);
    if (name == "x-powered-by")
      continue;  // TODO: server_tokens ?
    else if (name == "status") {
      std::istringstream ss(line);
      std::string status;
      std::getline(ss, status, ' ');
      _response.setStatus(status);
      std::getline(ss, status);
      _response.setReason(status);
    } else
      _response.setHeader(name, line);
  }

  if (_response.getBody()->good() == false) return OnCgiError();
  _response.setHeader("Content-Length", toString(bodySize));
  _response.setReady();
  sendResponse();
}

void Http::OnCgiError() {
  processError("500", "Internal Server Error");
  sendResponse();
}

void Http::processRequest() {
  if (_virtualHost == NULL) return processError("500", "Internal Server Error");
  if (_virtualHost->getContext().exists("server_name", true))
    accessLog_g.write("VirtualHost: " + _virtualHost->getContext().getDirective(
                                            "server_name", true)[0][0],
                      DEBUG);

  _uri = _request.getUri().getPath();
  _context = _virtualHost->matchLocation(_uri);
  if (_context == NULL) return processError("500", "Internal Server Error");

  std::string contextUri = getContextArgs();
  if (Log::getLevel() >= DEBUG)
    accessLog_g.write("Context URI: '" + contextUri + "'", DEBUG);

  if (isMethodValid() == false) {
    processError("405", "Method Not Allowed");
    _response.setHeader("Allow", concatenate(getAllowedMethods(), ", "));
    return;
  }

  if (_request.getMethod() == "OPTIONS") return processOptions(_uri);

  if (_context->exists("redirect"))
    return processRedirect(_context->getDirective("redirect")[0][0]);

  if (_context->exists("alias"))
    _uri = getContextPath("alias") + _uri.substr(contextUri.size());
  accessLog_g.write("Resource URI: '" + _uri + "'", DEBUG);

  if (_request.getMethod() == "DELETE") return processDelete(_uri);

  if (_request.getMethod() == "PUT" || _request.getMethod() == "POST")
    return processBodyRequest();

  return processFile(_uri);
}

void Http::processFile(std::string uri) {
  File file(_context->getDirective("root", true)[0][0] + uri);

  addIndexToPath(file, uri);
  checkResourceValidity(file, uri);
  if (_response.isReady()) return;

  std::string cgiPath = getCgiPath(file.getExtension());
  if (cgiPath.size() > 0)
    return processCgi(uri, file, cgiPath);
  else if (_request.getMethod() == "POST") {
    processError("405", "Method Not Allowed");
    std::vector<std::string> allowed = getAllowedMethods();
    allowed.erase(std::find(allowed.begin(), allowed.end(), "POST"));
    _response.setHeader("Allow", concatenate(allowed, ", "));
    return;
  }

  // Load the file
  _response.init("HTTP/1.1", "200", "OK");
  _response.setBody(new std::ifstream(file.getPath().c_str()));
  int bodySize = file.size();
  if (_response.getBody()->good() == false || bodySize < 0)
    return processError("500", "Internal Server Error");
  _response.setHeader("Content-Length", toString(bodySize));
  _response.setHeader("Last-modified",
                      file.lastModified("%a, %d %b %Y %H:%M:%S"));

  // Set mime type
  std::string mimeType = VirtualHost::getMimeType(file.getExtension());
  if (!mimeType.empty()) _response.setHeader("Content-Type", mimeType);
  _response.setReady();
}

void Http::processCgi(std::string const &uri, File const &file,
                      std::string const &cgiPathname) {
  (void)uri;
  // std::string root = _context->getDirective("root", true)[0][0]; // TODO:
  // remove if not needed
  std::string pathname = file.getPath();
  if (!startsWith(pathname, "/")) pathname.insert(0, cwd_g + "/");

  // if (!startsWith(root, "/"))
  //  root.insert(0, cwd);  // TODO: remove if not needed
  std::vector<std::string> env;

  // const values:
  env.push_back("GATEWAY_INTERFACE=CGI/1.1");
  env.push_back("SERVER_SOFTWARE=" WEBSERV_ID);
  env.push_back("SERVER_PROTOCOL=HTTP/" HTTP_VERSION);

  // request specific values:

  // Required amongst others for wordpress to function
  // env.push_back("DOCUMENT_ROOT=" + root); // TODO: needed?
  env.push_back("SCRIPT_FILENAME=" + pathname);

  const Uri &uriRef = _request.getUri();
  std::string requestUri = uriRef.getPath();
  if (uriRef.getQuery() != "") requestUri += "?" + uriRef.getQuery();
  env.push_back("REQUEST_URI=" + requestUri);

  env.push_back("HTTP_HOST=" + _request.getHeader("Host"));
  env.push_back("REQUEST_METHOD=" + _request.getMethod());
  env.push_back("QUERY_STRING=" + _request.getUri().getQuery());
  env.push_back("REDIRECT_STATUS=200");

  // Required amongst others to comply with CGI/1.1
  if (_request.getMethod() == "POST") {
    env.push_back("CONTENT_LENGTH=" +
                  _request.getHeader(
                      "Content-length"));  // TODO: What if request is chunked?
    if (_request.getHeader("Content-type") != "")
      env.push_back("CONTENT_TYPE=" + _request.getHeader("Content-type"));
  }
  std::string servername;
  if (_context->exists("server_name", true))
    servername = _context->getDirective("server_name", true)[0][0];
  else
    servername = _request.getHeader("Host");
  env.push_back("SERVER_NAME=" + servername);
  env.push_back("SERVER_PORT=" + toString<in_port_t>(host.port()));
  env.push_back("REMOTE_ADDR=" + client.str());
  env.push_back("REMOTE_PORT=" + toString<in_port_t>(client.port()));

  // Optional stuff to increase functionality
  env.push_back("HTTP_COOKIE=" + _request.getHeader("Cookie"));
  env.push_back("HTTP_USER_AGENT=" + _request.getHeader("User-Agent"));

  // TODO: Not used atm
  // env.push_back("PATH_TRANSLATED=" + File(pathname).getDir());
  // env.push_back("SERVER_ADDR=" + host.str());
  // env.push_back("SCRIPT_NAME=" + cgiTESTPATH); // cgiTESTPATH being an
  // absolute path to the cgi executable TODO: Check if it needs to be
  // implemented this way instead of SCRIPT_FILENAME

  std::vector<std::string> arg;
  arg.push_back(pathname);  // TODO: Check how to make input file selection work
                            // this way without having to use env variables
  runCGI(cgiPathname, arg, env);
  if (_request.getMethod() != "POST") cgiCloseSendPipe();
  _response.clear();
}

void Http::processBodyRequest() {
  if (_request.getHeader("Content-Range") != "")
    return processError("501", "Not Implemented");

  _currBodySize = 0;
  if (_request.getHeader("Transfer-Encoding") != "") {
    if (_request.getHeader("Transfer-Encoding") != "chunked")
      return processError("501", "Not Implemented");
    return setReadState(CHUNK_SIZE);
  }

  // Check if body size is specified
  std::string bodySizeStr = _request.getHeader("Content-Length");
  if (bodySizeStr.empty()) return processError("411", "Length Required");
  _expectedBodySize = fromString<size_t>(bodySizeStr);

  // Check if body size is too large
  if (isBodySizeValid(_expectedBodySize) == false)
    return processError("413", "Request Entity Too Large", true);

  bodySize = std::min(static_cast<size_t>(BUFFER_SIZE), _expectedBodySize);
  setReadState(BODY);

  if (_request.getMethod() == "POST")
    return processFile(_uri);
  else if (_request.getMethod() == "PUT") {
    std::string path = _context->getDirective("root", true)[0][0] + _uri;
    _newFile = !File(path).exists();

    if (!File(File(path).getDir()).exists())
      return processError("404", "Not found");

    if (_file.is_open()) _file.close();
    _file.open(path.c_str(), std::ios::out | std::ios::binary);
    if (_file.good() == false)
      return processError("500", "Internal Server Error");
  }
}

void Http::processPutData(const std::string &data) {
  _file.write(data.c_str(), data.size());
  if (_file.good() == false)
    return processError("500", "Internal Server Error");
  _currBodySize += data.size();

  if (_request.getHeader("Transfer-Encoding") == "chunked") {
    if (data.size() == 0) return getPutResponse(_uri);
    return setReadState(CHUNK_SIZE);
  }

  if (_currBodySize >= _expectedBodySize) return getPutResponse(_uri);
  bodySize = std::min((size_t)BUFFER_SIZE, _expectedBodySize - _currBodySize);
}

void Http::processPostData(std::string &data) {
  cgiSend(data);
  _currBodySize += data.size();
  if (_request.getHeader("Transfer-Encoding") == "chunked") {
    if (data.size() == 0) return cgiCloseSendPipe();
    return setReadState(CHUNK_SIZE);
  }

  if (_currBodySize >= _expectedBodySize) {
    cgiCloseSendPipe();
    return;
  }
  bodySize = std::min((size_t)BUFFER_SIZE, _expectedBodySize - _currBodySize);
}

void Http::getPutResponse(std::string uri) {
  if (_newFile)
    _response.init("HTTP/1.1", "201", "Created");
  else
    _response.init("HTTP/1.1", "204", "No Content");
  _response.setHeader("Location", getAbsoluteUri(uri));
  _response.setReady();
}

void Http::processOptions(std::string uri) {
  _response.init("HTTP/1.1", "200", "OK");
  _response.setHeader("Allow",
                      concatenate(getAllowedMethods(uri != "/*"), ", "));
  _response.setReady();
}

void Http::processDelete(std::string uri) {
  std::string path = _context->getDirective("root", true)[0][0] + uri;
  File file(path);

  if (!file.exists()) return processError("404", "Not Found");
  if (file.dir()) return processError("403", "Forbidden");
  if (!file.readable()) return processError("403", "Forbidden");
  if (std::remove(file.getPath().c_str()) != 0)
    return processError("500", "Internal Server Error");
  _response.init("HTTP/1.1", "204", "No Content");
  _response.setReady();
}

void Http::processAutoindex(std::string uri) {
  _response.init("HTTP/1.1", "200", "OK");
  _response.setBody(new std::stringstream("<html>\r\n<head><title>Index of " +
                                          uri + "</title></head>\r\n<body>"));
  std::stringstream *body = (std::stringstream *)_response.getBody();
  *body << "<h1>Index of " + uri + "</h1><hr><pre><a href=\"../\">../</a>\r\n";

  // Get list of files in directory
  std::string path = _context->getDirective("root", true)[0][0] + uri;
  std::set<std::string> files;
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

  _response.setHeader("Content-Length", toString(getStreamBufferSize(*body)));
  _response.setHeader("Content-Type", "text/html");
  _response.setReady();
}

void Http::processRedirect(std::string uri) {
  _response.init("HTTP/1.1", "301", "Moved Permanently");
  _response.setHeader("Location", getAbsoluteUri(uri));
  _response.setReady();
}

void Http::processError(std::string code, std::string reason, bool close) {
  _response.init("HTTP/1.1", code, reason);

  if (_virtualHost && _virtualHost->getContext().exists("error_page")) {
    std::vector<std::vector<std::string> > &pages =
        _virtualHost->getContext().getDirective("error_page");

    // Search for matching custom error page
    for (size_t i = 0; i < pages.size(); i++) {
      if (pages[i][0] != code) continue;
      std::string path =
          _context->getDirective("root", true)[0][0] + pages[i][1];
      File file(path);
      if (file.exists() && file.file() && file.readable()) {
        _response.setBody(new std::ifstream(path.c_str()));
        size_t bodySize = file.size();
        if (_response.getBody()->good() == false || bodySize < 0)
          return processError("500", "Internal Server Error");
        _response.setHeader("Content-Length", toString(bodySize));

        // Set mime type
        std::string mimeType = VirtualHost::getMimeType(file.getExtension());
        if (!mimeType.empty()) _response.setHeader("Content-Type", mimeType);
      } else if (code != "404")
        return processError("404", "Not Found");
      break;
    }
  }

  // If no custom error page was found, use default
  if (_response.getBody() == NULL) {
    std::string body = getDefaultBody(code, reason);
    _response.setBody(new std::stringstream(body));
    _response.setHeader("Content-Type", "text/html");
    _response.setHeader("Content-Length", toString(body.size()));
  }
  if (close) _response.setHeader("Connection", "close");
  _response.setReady();
}

void Http::addIndexToPath(File &file, std::string &uri) {
  if (endsWith(uri, "/") && _context->exists("index", true)) {
    std::string path = file.getPath();
    std::vector<std::string> indexes = _context->getDirective("index", true)[0];
    for (size_t i = 0; i < indexes.size(); i++) {
      file = File(path + indexes[i]);
      if (file.exists() && file.file() && file.readable()) {
        uri += indexes[i];
        return;
      }
    }
    file = File(path);
  }
}

void Http::checkResourceValidity(const File &file, const std::string &uri) {
  if (!file.exists()) {
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
}

std::string Http::getDefaultBody(std::string code, std::string reason) const {
  return "<html>\r\n<head><title>" + code + " " + reason +
         "</title></head>\r\n<body>\r\n<center><h1>" + code + " " + reason +
         "</h1></center>\r\n<hr><center>" WEBSERV_ID
         "</center>\r\n</body>\r\n</"
         "html>\r\n";
}

void Http::sendResponse() {
  if (_response.isReady() == false) {
    accessLog_g.write("Trying to send response before it is ready", WARNING);
    return;
  }

  // Set default header values
  _response.setHeader("Server", WEBSERV_ID);
  if (_response.getHeader("Content-Length").empty())
    _response.setHeader("Content-Length", toString(0));
  if (_response.getHeader("Content-type").empty())
    _response.setHeader("Content-type", HTTP_DEFAULT_MIME);

  // https://datatracker.ietf.org/doc/html/rfc2616#section-14.18
  _response.setHeader("Date", getTime("%a, %d %b %Y %H:%M:%S"));

  // Check if connection should be kept alive
  if (_request.getHeader("Connection") != "close" &&
      _response.getHeader("Connection") == "")
    _response.setHeader("Connection", "keep-alive");

  // Send response
  send(new std::istringstream(_response.generateHead()));
  if (_request.getMethod() != "HEAD") send(_response.resetBody());

  // Reset class variables
  setReadState(STATUS);

  // Close connection if needed or asked for
  if (_response.getHeader("Connection") == "close" ||
      _request.getHeader("Connection") == "close")
    stopReceiving();

  accessLog_g.write(_log + " -> " + _response.getVersion() + " " +
                        _response.getStatus() + " " + _response.getReason() +
                        " " + _request.getHeader("User-Agent"),
                    INFO);
  _response.clear();
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
  std::string methods[] = HTTP_METHODS;
  for (size_t i = 0; i < sizeof(methods) / sizeof(std::string); i++)
    if (methods[i] == method) return true;
  return false;
}

bool Http::isHttpVersionValid(std::string version) const {
  if (startsWith(version, "HTTP/"))
    version.erase(0, std::string("HTTP/").size());
  std::vector<std::string> in = split(version, ".");
  if (in.size() < 1 || in.size() > 2) return false;

  std::vector<std::string> reqired = split(HTTP_VERSION, ".");
  if (reqired.size() < 1 || reqired.size() > 2 || in.size() != reqired.size())
    return false;

  for (size_t i = 0; i < in.size(); i++)
    if (fromString<size_t>(trimStart(in[i], "0")) !=
        fromString<size_t>(trimStart(reqired[i], "0")))
      return false;
  return true;
}

bool Http::isMethodValid() const {
  std::vector<std::string> allowedMethods = getAllowedMethods();
  for (size_t i = 0; i < allowedMethods.size(); i++)
    if (allowedMethods[i] == _request.getMethod()) return true;
  return false;
}

bool Http::isBodySizeValid(size_t size) const {
  size_t maxBodySize = MAX_CLIENT_BODY_SIZE;
  if (_context->exists("max_client_body_size", true))
    maxBodySize =
        toBytes(_context->getDirective("max_client_body_size", true)[0][0]);
  return size <= maxBodySize;
}

std::string Http::getCgiPath(std::string extension) const {
  if (!_context->exists("cgi", true)) return "";
  std::vector<std::vector<std::string> > &cgis =
      _context->getDirective("cgi", true);
  for (size_t i = 0; i < cgis.size(); i++)
    if (cgis[i][0] == extension) return cgis[i][1];
  return "";
}

std::vector<std::string> Http::getAllowedMethods(bool forUri) const {
  std::vector<std::string> ret;
  if (forUri == false) {
    std::string methods[] = HTTP_METHODS;
    for (size_t i = 0; i < sizeof(methods) / sizeof(std::string); i++)
      ret.push_back(methods[i]);
  } else if (_context->exists("allow", true))
    ret = _context->getDirective("allow", true)[0];
  else {
    std::string methods[] = HTTP_DEFAULT_METHODS;
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
