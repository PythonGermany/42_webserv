#include "Request.hpp"

#include <iostream>
Request::Request(int fd) {
  std::string request = "";
  char buffer[1024];
  int bytes_read = read(fd, buffer, 1024);
  request += std::string(buffer, bytes_read);
  if (bytes_read < 0) throw std::runtime_error("read error");
  std::string header = request.substr(0, request.find("\r\n\r\n"));
  body = request.substr(request.find("\r\n\r\n") + 4);

  std::string request_line = header.substr(0, header.find("\r\n"));
  std::string fields = header.substr(header.find("\r\n") + 2);

  method = request_line.substr(0, request_line.find(" "));
  uri = request_line.substr(request_line.find(" ") + 1);
  version = uri.substr(uri.find(" ") + 1);
  uri = uri.substr(0, uri.find(" "));

  while (fields.find("\r\n") != std::string::npos) {
    std::string field = fields.substr(0, fields.find("\r\n"));
    std::string key = field.substr(0, field.find(": "));
    std::string value = field.substr(field.find(": ") + 2);
    this->fields[key] = value;
    fields = fields.substr(fields.find("\r\n") + 2);
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