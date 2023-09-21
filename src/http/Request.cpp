#include "Request.hpp"

Request::Request() {}

Request::Request(const Request &rhs) { *this = rhs; }

Request &Request::operator=(const Request &rhs) {
  if (this == &rhs) return *this;
  _method = rhs._method;
  _uri = rhs._uri;
  _version = rhs._version;
  _headers = rhs._headers;
  _body = rhs._body;
  return *this;
}

Request::~Request() {}

void Request::setBody(std::string body) { _body = body; }

std::string Request::getMethod() const { return _method; }

Uri Request::getUri() const { return _uri; }

std::string Request::getVersion() const { return _version; }

std::string Request::getHeader(std::string key) const {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  std::map<std::string, std::string>::const_iterator it = _headers.find(key);
  if (it == _headers.end()) return "";
  return it->second;
}

std::string Request::getBody() const { return _body; }

int Request::parseHead(std::string msg) {
  // Find head delimiter
  size_t pos = msg.find("\r\n");
  if (pos == std::string::npos) return 1;

  // Parse request line
  std::string requestLine = msg.substr(0, pos);
  std::vector<std::string> requestLineTokens = split(requestLine, " ");
  if (requestLineTokens.size() != 3) return 1;
  _method = requestLineTokens[0];
  _uri = Uri(requestLineTokens[1]);
  _version = requestLineTokens[2];

  // Parse headers
  while (pos != std::string::npos) {
    pos += 2;
    size_t end = msg.find("\r\n", pos);
    std::string line = msg.substr(pos, end - pos);
    pos = end;
    if (line.empty()) break;
    size_t colon = line.find(":");
    if (colon == std::string::npos) return 1;
    std::string key = line.substr(0, colon);
    std::string value = line.substr(colon + 1);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    trimStart(value, " \t");
    if (_headers.find(key) != _headers.end())
      _headers[key] += ", " + value;
    else
      _headers[key] = value;
  }
  return 0;
}

bool Request::isValid() const {
  if (_uri.pathOutOfBound() || !startsWith(_version, "HTTP/")) return false;
  if (_headers.find("host") == _headers.end()) return false;
  return true;
}