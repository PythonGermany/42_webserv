#include "Http.hpp"

Http::Http(Address const &client, Address const &host)
    : AConnection(host, client) {
  setReadState(REQUEST_LINE);
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
  try {
    accessLog_g.write(toString<Address &>(host) +
                          " -> delete: " + toString<Address &>(client),
                      DEBUG);
  } catch (...) {
  }
}

void Http::OnRequestRecv(std::string msg) {
  accessLog_g.write("HTTP status: '" + msg + "'", VERBOSE);

  // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-6
  if (msg.empty()) return;

  _request = Request();
  bool parseRet = _request.parseRequestLine(msg);
  _log = toString<Address &>(client) + ": " + _request.getMethod() + " " +
         _request.getUri().generate() + " " + _request.getVersion() + " -> " +
         toString<Address &>(host);
  parseRet |= _request.getUri().decode();
  accessLog_g.write("Decoded URI: " + _request.getUri().generate(), DEBUG);

  bool validPath = !_request.getUri().resolveDots();
  accessLog_g.write("Resolved URI: " + _request.getUri().generate(), DEBUG);

  const Uri &uriRef = _request.getUri();
  validPath &= startsWith(uriRef.getPath(), "/") ||
               (_request.isMethod("OPTIONS") && uriRef.getPath() == "*");

  if (parseRet || !validPath)
    processError("400", "Bad Request", true);
  else if (isHttpVersionValid(_request.getVersion()) == false) {
    processError("505", "HTTP Version Not Supported", true);
    _response.setHeader("Upgrade", PROTOCOL "/" HTTP_VERSION);
    _response.setHeader("Connection", "Upgrade");
  } else if (isMehodImplemented(_request.getMethod()) == false)
    processError("501", "Not Implemented", true);
  if (_response.isReady()) return sendResponse();
  setReadState(HEAD);
}

void Http::OnHeadRecv(std::string msg) {
  accessLog_g.write("HTTP head: '" + msg + "'", VERBOSE);

  // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-8
  if (_request.parseHeaderFields(msg))
    processError("400", "Bad Request", true);
  else {
    std::string requestHost = _request.getHeaderField("Host");
    if (_request.getUri().getHost().size() > 0)
      requestHost = _request.getUri().getHost();

    if (_request.getHeaderField("Host").size() > 0) {
      _virtualHost = VirtualHost::matchVirtualHost(host, requestHost);
      processRequest();
    } else
      processError("400", "Bad Request", true);
  }
  if (_response.isReady()) sendResponse();
}

void Http::OnChunkSizeRecv(std::string msg) {
  accessLog_g.write("HTTP chunk size: '" + msg + "'", VERBOSE);

  msg.substr(0, msg.find(';'));
  bodySize = 0;
  if (std::sscanf(msg.substr(0, msg.size()).c_str(), "%lx", &bodySize) == EOF) {
    errorLog_g.write("OnChunkSizeRecv(): sscanf failure", DEBUG, BRIGHT_RED);
    processError("500", "Internal server error", true);
  } else if (isBodySizeValid(_currBodySize + bodySize) == false)
    processError("413", "Request Entity Too Large", true);
  if (_response.isReady()) return sendResponse();
  if (bodySize == 0)
    setReadState(TRAILER);
  else {
    bodySize += 2;
    setReadState(BODY);
  }
}

void Http::OnTrailerRecv(std::string msg) {
  accessLog_g.write("HTTP trailer: '" + msg + "'", VERBOSE);

  if (msg.empty()) {
    if (_request.isMethod("PUT"))
      getPutResponse(_uri);
    else if (_request.isMethod("POST")) {
      processCgi(toString(_currBodySize));
      cgiSend(cgiChunkedBody);
      cgiCloseSendPipe();
    } else
      errorLog_g.write(
          "OnTrailerRecv() processed unknown method: " + _request.getMethod(),
          ERROR);
    _request.removeHeaderValue("Transfer-Encoding", "chunked");
  } else if (_request.parseHeaderFields(msg))
    processError("400", "Bad Request", true);
  if (_response.isReady()) sendResponse();
}

void Http::OnBodyRecv(std::string msg) {
  if (_request.hasHeaderFieldValue("Transfer-Encoding", "chunked")) {
    if (!endsWith(msg, "\r\n")) return processError("400", "Bad Request", true);
    msg.erase(msg.size() - 2, 2);
  }
  accessLog_g.write("HTTP body: '" + msg + "'", VERBOSE);

  if (_request.isMethod("PUT"))
    processPutData(msg);
  else if (_request.isMethod("POST"))
    processPostData(msg);
  if (_response.isReady()) sendResponse();
}

void Http::OnCgiRecv(std::string msg) {
  accessLog_g.write("CGI out: \"" + msg + "\"", VERBOSE);
  _response.init(PROTOCOL "/" HTTP_VERSION, "200", "OK");
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
    if (name == "status") {
      std::istringstream ss(line);
      std::string status;
      std::getline(ss, status, ' ');
      _response.setStatus(status);
      std::getline(ss, status);
      _response.setReason(status);
    } else if (name == "set-cookie") {
      _response.setCookie(line);
    } else
      _response.setHeader(name, line);
  }

  if (_response.getBody()->good() == false) return OnCgiError();
  _response.setHeader("Content-Length", toString(bodySize), true);
  _response.setReady();
  sendResponse();
}

void Http::OnCgiError() {
  processError("500", "Internal Server Error");
  sendResponse();
}

void Http::processRequest() {
  if (_virtualHost == NULL) return processError("500", "Internal Server Error");

  _uri = _request.getUri().getPath();
  _context = _virtualHost->matchLocation(_uri);
  if (_context == NULL) return processError("500", "Internal Server Error");

  std::string contextUri = getContextArgs();
  accessLog_g.write("Context URI: '" + contextUri + "'", DEBUG);

  if (isMethodValid() == false) {
    bool bodyRequest = _request.isMethod("POST") || _request.isMethod("PUT");
    processError("405", "Method Not Allowed", bodyRequest);
    _response.setHeader("Allow", concatenate(getAllowedMethods(), ", "));
    return;
  }

  if (_request.isMethod("OPTIONS")) return processOptions(_uri);

  if (_context->exists("redirect"))
    return processRedirect(_context->getDirective("redirect")[0][0]);

  if (_context->exists("alias"))
    _uri = getContextPath("alias") + _uri.substr(contextUri.size());
  accessLog_g.write("Resource URI path: '" + _uri + "'", DEBUG);

  if (_request.isMethod("DELETE")) return processDelete(_uri);

  if (_request.isMethod("PUT") || _request.isMethod("POST"))
    return processBodyRequest();

  return processFile(_uri);
}

void Http::processFile(std::string uri) {
  File file(_context->getDirective("root", true)[0][0] + uri);

  addIndexToPath(file, uri);
  checkResourceValidity(file, uri);
  if (_response.isReady()) return;

  cgiProgramPathname = getCgiPath(file.getExtension());
  if (cgiProgramPathname.size() > 0) {
    cgiFilePathname = file.getPath();
    if (_request.isMethod("POST") &&
        _request.hasHeaderFieldValue("Transfer-Encoding", "chunked")) {
      cgiChunkedBody.clear();
      setReadState(CHUNK_SIZE);
      return;
    }
    return processCgi();
  } else if (_request.isMethod("POST")) {
    processError("405", "Method Not Allowed", true);
    std::vector<std::string> allowed = getAllowedMethods();
    allowed.erase(std::find(allowed.begin(), allowed.end(), "POST"));
    _response.setHeader("Allow", concatenate(allowed, ", "));
    return;
  }

  // Load the file
  _response.init(PROTOCOL "/" HTTP_VERSION, "200", "OK");
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

void Http::processCgi(std::string contentLength) {
  std::string pathname(cgiFilePathname);
  if (!startsWith(pathname, "/")) pathname.insert(0, cwd_g);

  std::vector<std::string> env;
  // const values:
  env.push_back("GATEWAY_INTERFACE=CGI/1.1");
  env.push_back("SERVER_SOFTWARE=" WEBSERV_ID);
  env.push_back("SERVER_PROTOCOL=" PROTOCOL "/" HTTP_VERSION);

  // request specific values:

  // Required amongst others for wordpress to function
  env.push_back("SCRIPT_FILENAME=" + pathname);

  const Uri &uriRef = _request.getUri();
  std::string requestUri = uriRef.getPath();
  if (uriRef.getQuery() != "") requestUri += "?" + uriRef.getQuery();
  env.push_back("REQUEST_URI=" + requestUri);

  env.push_back("HTTP_HOST=" + _request.getHeaderField("Host"));
  env.push_back("REQUEST_METHOD=" + _request.getMethod());
  env.push_back("QUERY_STRING=" + _request.getUri().getQuery());
  env.push_back("REDIRECT_STATUS=200");

  // Required amongst others to comply with CGI/1.1
  env.push_back("SCRIPT_NAME=" + pathname);
  if (_request.isMethod("POST")) {
    if (contentLength.empty())
      env.push_back("CONTENT_LENGTH=" +
                    _request.getHeaderField("Content-length"));
    else
      env.push_back("CONTENT_LENGTH=" + contentLength);
    if (_request.getHeaderField("Content-type") != "")
      env.push_back("CONTENT_TYPE=" + _request.getHeaderField("Content-type"));
  }

  env.push_back("SERVER_NAME=" + _request.getHeaderField("Host"));
  env.push_back("SERVER_PORT=" + toString<in_port_t>(host.port()));
  std::ostringstream oss;
  oss << "REMOTE_ADDR=" << client;
  env.push_back(oss.str());

  // Optional stuff to increase functionality
  env.push_back("HTTP_COOKIE=" + _request.getHeaderField("Cookie"));
  env.push_back("HTTP_USER_AGENT=" + _request.getHeaderField("User-Agent"));

  runCGI(cgiProgramPathname, std::vector<std::string>(1, pathname), env);
  if (_request.isMethod("POST") == false) cgiCloseSendPipe();
  _response.clear();
}

void Http::processBodyRequest() {
  if (_request.getHeaderField("Content-Range") != "")
    return processError("501", "Not Implemented");

  _currBodySize = 0;
  if (_request.getHeaderField("Transfer-Encoding") != "") {
    if (_request.getHeaderField("Transfer-Encoding") != "chunked")
      return processError("501", "Not Implemented");
    setReadState(CHUNK_SIZE);
  } else {
    // Check if body size is specified
    std::string bodySizeStr = _request.getHeaderField("Content-Length");
    if (bodySizeStr.empty()) return processError("411", "Length Required");
    _expectedBodySize = fromString<size_t>(bodySizeStr);

    // Check if body size is too large
    if (isBodySizeValid(_expectedBodySize) == false)
      return processError("413", "Request Entity Too Large", true);

    bodySize = std::min(static_cast<size_t>(BUFFER_SIZE), _expectedBodySize);
    setReadState(BODY);
  }

  if (_request.isMethod("POST"))
    return processFile(_uri);
  else if (_request.isMethod("PUT")) {
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

  if (_request.hasHeaderFieldValue("Transfer-Encoding", "chunked"))
    return setReadState(CHUNK_SIZE);

  if (_currBodySize >= _expectedBodySize) return getPutResponse(_uri);
  bodySize = std::min(static_cast<size_t>(BUFFER_SIZE),
                      _expectedBodySize - _currBodySize);
}

void Http::processPostData(const std::string &data) {
  _currBodySize += data.size();
  if (_request.hasHeaderFieldValue("Transfer-Encoding", "chunked")) {
    cgiChunkedBody.append(data);
    return setReadState(CHUNK_SIZE);
  }
  cgiSend(data);
  if (_currBodySize >= _expectedBodySize) {
    cgiCloseSendPipe();
    return;
  }
  bodySize = std::min(static_cast<size_t>(BUFFER_SIZE),
                      _expectedBodySize - _currBodySize);
}

void Http::getPutResponse(std::string uri) {
  if (_newFile)
    _response.init(PROTOCOL "/" HTTP_VERSION, "201", "Created");
  else
    _response.init(PROTOCOL "/" HTTP_VERSION, "204", "No Content");
  _response.setHeader("Location", getAbsoluteUri(uri));
  _response.setReady();
}

void Http::processOptions(std::string uri) {
  _response.init(PROTOCOL "/" HTTP_VERSION, "200", "OK");
  _response.setHeader("Allow",
                      concatenate(getAllowedMethods(uri != "*"), ", "));
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
  _response.init(PROTOCOL "/" HTTP_VERSION, "204", "No Content");
  _response.setReady();
}

void Http::processAutoindex(std::string uri) {
  _response.init(PROTOCOL "/" HTTP_VERSION, "200", "OK");
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
  _response.init(PROTOCOL "/" HTTP_VERSION, "301", "Moved Permanently");
  _response.setHeader("Location", getAbsoluteUri(uri));
  _response.setReady();
}

void Http::processError(std::string code, std::string reason, bool close) {
  _response.init(PROTOCOL "/" HTTP_VERSION, code, reason);

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
  if (!file.exists()) return processError("404", "Not Found");
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
    _response.setHeader("Content-Length", "0");
  if (_response.getHeader("Content-type").empty())
    _response.setHeader("Content-type", HTTP_DEFAULT_MIME);

  // https://datatracker.ietf.org/doc/html/rfc2616#section-14.18
  _response.setHeader("Date", getTime("%a, %d %b %Y %H:%M:%S"));

  // Check if connection should be kept alive
  if (_request.getHeaderField("Connection") != "close" &&
      _response.getHeader("Connection") == "")
    _response.setHeader("Connection", "keep-alive", true);

  // Send response
  send(new std::istringstream(_response.generateHead()));
  if (_request.isMethod("HEAD") == false) send(_response.resetBody());

  // Reset class variables
  setReadState(REQUEST_LINE);

  // Close connection if needed or asked for
  if (_response.getHeader("Connection") == "close" ||
      _request.hasHeaderFieldValue("Connection", "close"))
    stopReceiving();

  accessLog_g.write(_log + " -> " + _response.getVersion() + " " +
                        _response.getStatus() + " " + _response.getReason() +
                        " " + _request.getHeaderField("User-Agent"),
                    INFO);
  _response.clear();
}

std::string Http::getAbsoluteUri(std::string uri) const {
  Uri ret;

  ret.load(uri);
  if (ret.getScheme().empty()) ret.setScheme("http");
  if (ret.getHost().empty()) {
    std::string host = _request.getHeaderField("Host");
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
  if (startsWith(version, PROTOCOL "/"))
    version.erase(0, std::string(PROTOCOL "/").size());
  std::vector<std::string> in = split<std::vector<std::string> >(version, ".");
  if (in.size() < 1 || in.size() > 2) return false;

  std::vector<std::string> reqired =
      split<std::vector<std::string> >(HTTP_VERSION, ".", true);
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
  std::vector<std::string>::const_iterator it = allowedMethods.begin();
  for (; it != allowedMethods.end(); ++it)
    if (_request.getMethod() == *it) return true;
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
