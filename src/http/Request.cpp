#include "Request.hpp"

Request::Request() : _isValid(false) {}

Request::Request(const Request &rhs) { *this = rhs; }

Request &Request::operator=(const Request &rhs) {
  if (this == &rhs) return *this;
  _isValid = rhs._isValid;
  _method = rhs._method;
  _uri = rhs._uri;
  _version = rhs._version;
  _headers = rhs._headers;
  _body = rhs._body;
  return *this;
}

Request::~Request() {}

bool Request::isValid() const { return _isValid; }

std::string Request::getMethod() const { return _method; }

std::string Request::getUri() const { return _uri; }

std::string Request::getVersion() const { return _version; }

std::map<std::string, std::string> Request::getHeaders() const {
  return _headers;
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
  if (requestLineTokens.size() != 3) {
    std::cout << "request line has " << requestLineTokens.size()
              << " tokens instead of 3" << std::endl;
    return;
  }
  _method = requestLineTokens[0];
  _uri = requestLineTokens[1];
  _version = requestLineTokens[2];
  pos += 2;
  pos = msg.find("\r\n", pos);
  while (pos != std::string::npos && msg.size() > pos + 2) {
    size_t pos2 = msg.find(":", pos);
    if (pos2 == std::string::npos) {
      std::cout << "no : found" << std::endl;
      return;
    }
    std::string key = msg.substr(pos, pos2 - pos);
    pos2 += 2;
    pos = msg.find("\r\n", pos2);
    if (pos == std::string::npos) {
      std::cout << "no \\r\\n found" << std::endl;
      return;
    }
    std::string value = msg.substr(pos2, pos - pos2);
    _headers[key] = value;
    pos += 2;
  }
  _isValid = true;
}