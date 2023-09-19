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

std::string Request::getMethod() const { return _method; }

std::string Request::getUri() const { return _uri; }

std::string Request::getVersion() const { return _version; }

std::map<std::string, std::string> Request::getHeaders() const {
  return _headers;
}

std::string Request::getHeader(std::string key) const {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  std::map<std::string, std::string>::const_iterator it = _headers.find(key);
  if (it == _headers.end()) return "";
  return it->second;
}

std::string Request::getBody() const { return _body; }

#include <iostream>

void Request::parseHead(std::string msg) {
  size_t pos = msg.find("\r\n");
  if (pos == std::string::npos) {
    std::cout << "line: no \\r\\n found" << std::endl;
    return;
  }
  std::string requestLine = msg.substr(0, pos);
  std::vector<std::string> requestLineTokens = split(requestLine, " ");
  if (requestLineTokens.size() != 3) return;
  _method = requestLineTokens[0];
  _uri = requestLineTokens[1];
  _version = requestLineTokens[2];
  while (pos != std::string::npos) {
    pos += 2;
    size_t pos2 = msg.find(":", pos);
    if (pos2 == std::string::npos) return;
    std::string key = msg.substr(pos, pos2 - pos);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    pos2 += 2;
    pos = msg.find("\r\n", pos2);
    if (pos == std::string::npos) return;
    std::string value = msg.substr(pos2, pos - pos2);
    _headers[key] += value;
  }
}

bool Request::isValid() const {
  if (_method.empty() || _uri.empty() || _version.empty()) return false;
  if (!startsWith(_version, "HTTP/")) return false;
  if (_headers.find("host") == _headers.end()) return false;
  return true;
}