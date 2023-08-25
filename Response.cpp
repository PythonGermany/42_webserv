#include "Response.hpp"

Response::Response() : status(""), body("") {}

Response::Response(std::string status_code, std::string message) {
  status = "HTTP/1.1 " + status_code + " " + message + "\r\n";
  body = "";
}

Response::Response(const Response& other) { *this = other; }

Response& Response::operator=(const Response& other) {
  if (this == &other) return *this;
  status = other.status;
  fields = other.fields;
  body = other.body;
  return *this;
}

Response::~Response() {}

void Response::set_field(std::string key, std::string value) {
  fields[key] = value;
}

void Response::set_body(int fd) {
  char buffer[1024];
  int bytes_read = read(fd, buffer, 1024);
  while (bytes_read > 0) {
    body += std::string(buffer, bytes_read);
    bytes_read = read(fd, buffer, 1024);
  }
  if (bytes_read < 0) throw std::runtime_error("read error");
}

void Response::set_body(std::string body) { this->body = body; }

std::string Response::get() const {
  std::string response = status;
  for (std::map<std::string, std::string>::const_iterator it = fields.begin();
       it != fields.end(); ++it) {
    response += it->first + ": " + it->second + "\r\n";
  }
  response += "\r\n" + body;
  return response;
}