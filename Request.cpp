#include "Request.hpp"

#include <iostream>

Request::Request() {}

Request::Request(int fd) {
  std::string request = "";
  char buffer[1024];
  int bytes_read = read(fd, buffer, 1024);
  request += std::string(buffer, bytes_read);
  if (bytes_read < 0) throw std::runtime_error("read error");
  size_t index = request.find("\r\n\r\n");
  if (index == std::string::npos) throw std::runtime_error("invalid request");
  std::string header = request.substr(0, index);
  body = request.substr(index + 4);

  index = header.find("\r\n");
  if (index == std::string::npos) throw std::runtime_error("invalid request");
  std::string request_line = header.substr(0, index);
  std::string fields = header.substr(index + 2);

  index = request_line.find(" ");
  if (index == std::string::npos) throw std::runtime_error("invalid request");
  method = request_line.substr(0, index);
  uri = request_line.substr(index + 1);
  index = uri.find(" ");
  if (index == std::string::npos) throw std::runtime_error("invalid request");
  version = uri.substr(index + 1);
  uri = uri.substr(0, index);

  index = fields.find("\r\n");
  while (index != std::string::npos) {
    std::string field = fields.substr(0, index);
    size_t separator = field.find(": ");
    if (separator == std::string::npos)
      throw std::runtime_error("invalid request");
    std::string key = field.substr(0, separator);
    std::string value = field.substr(separator + 2);
    this->fields[key] = value;
    fields = fields.substr(index + 2);
    index = fields.find("\r\n");
  }
}

Request::~Request() {}

std::string Request::get_method(void) const { return method; }

std::string Request::get_uri(void) const { return uri; }

std::string Request::get_version(void) const { return version; }

std::string Request::get_field(std::string key) const {
  std::map<std::string, std::string>::const_iterator it = fields.find(key);
  if (it == fields.end()) throw std::runtime_error("field not found");
  return it->second;
}

std::string Request::get_body(void) const { return body; }