#include "Request.hpp"

Request::Request() {}

Request::Request(int fd) {
  std::string request = "";
  char buffer[1024];
  int bytes_read = read(fd, buffer, 1024);
  if (bytes_read < 0) throw std::runtime_error("read error");
  request += std::string(buffer, bytes_read);

  std::string line = request.substr(0, request.find("\r\n"));
  std::vector<std::string> tokens = split(line, " ");
  if (tokens.size() != 3) throwException("invalid request line");
  _method = tokens[0];
  _uri = tokens[1];
  _version = tokens[2];
  request = request.substr(request.find("\r\n") + 2);

  _fields["Host"] = "localhost";
}

Request::Request(const Request& other) { *this = other; }

Request& Request::operator=(const Request& other) {
  if (this == &other) return *this;
  _method = other._method;
  _uri = other._uri;
  _version = other._version;
  _fields = other._fields;
  _body = other._body;
  return *this;
}

Request::~Request() {}

std::string Request::method(void) const { return _method; }

std::string& Request::uri(void) { return _uri; }

std::string Request::version(void) const { return _version; }

std::string Request::field(std::string key) const {
  std::map<std::string, std::string>::const_iterator it = _fields.find(key);
  if (it == _fields.end()) throwException("field not found");
  return it->second;
}

std::string Request::body(void) const { return _body; }

void Request::throwException(std::string msg) const {
  throw std::runtime_error("Request: " + msg);
}