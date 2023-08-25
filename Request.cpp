#include "Request.hpp"

Request::Request() {}

Request::Request(int fd) {
  std::string request = "";
  char buffer[1024];
  int bytes_read = read(fd, buffer, 1024);
  request += std::string(buffer, bytes_read);
  if (bytes_read < 0) throw std::runtime_error("read error");
  size_t index = request.find("\r\n\r\n");
  if (index == std::string::npos) throw_error("header not found");
  std::string header = request.substr(0, index);
  _body = request.substr(index + 4);

  index = header.find("\r\n");
  if (index == std::string::npos) throw_error("request line not found");
  std::string request_line = header.substr(0, index);
  std::string fields = header.substr(index + 2);

  index = request_line.find(" ");
  if (index == std::string::npos) throw_error("method not found");
  _method = request_line.substr(0, index);
  _uri = request_line.substr(index + 1);
  index = _uri.find(" ");
  if (index == std::string::npos) throw_error("uri/version not found");
  _version = _uri.substr(index + 1);
  _uri = _uri.substr(0, index);

  index = fields.find("\r\n");
  while (index != std::string::npos) {
    std::string field = fields.substr(0, index);
    size_t separator = field.find(": ");
    if (separator == std::string::npos)
      throw std::runtime_error("invalid request");
    std::string key = field.substr(0, separator);
    std::string value = field.substr(separator + 2);
    _fields[key] = value;
    fields = fields.substr(index + 2);
    index = fields.find("\r\n");
  }
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

std::string Request::uri(void) const { return _uri; }

std::string Request::version(void) const { return _version; }

std::string Request::field(std::string key) const {
  std::map<std::string, std::string>::const_iterator it = _fields.find(key);
  if (it == _fields.end()) throw std::runtime_error("field not found");
  return it->second;
}

std::string Request::body(void) const { return _body; }

void Request::throw_error(const std::string& error) {
  throw std::runtime_error("request: " + error);
}