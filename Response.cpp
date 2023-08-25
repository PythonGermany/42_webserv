#include "Response.hpp"

Response::Response() : status(""), _body("") {}

Response::Response(std::string status_code, std::string message) {
  status = "HTTP/1.1 " + status_code + " " + message + "\r\n";
  _body = "<html><title>" + status_code + " " + message +
          "</title><body><center><h1>" + status_code + " " + message +
          "</h1></center></body></html>";
}

Response::Response(const Response& other) { *this = other; }

Response& Response::operator=(const Response& other) {
  if (this == &other) return *this;
  status = other.status;
  fields = other.fields;
  _body = other._body;
  return *this;
}

Response::~Response() {}

void Response::set_field(std::string key, std::string value) {
  fields[key] = value;
}

void Response::load_body(std::string path) {
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) throw std::runtime_error("response: open error");

  char buffer[1024];
  int bytes_read = read(fd, buffer, 1024);
  std::string body = "";
  while (bytes_read > 0) {
    body += std::string(buffer, bytes_read);
    bytes_read = read(fd, buffer, 1024);
  }
  close(fd);
  if (bytes_read < 0) throw std::runtime_error("response: read error");
  _body = body;
}

void Response::send(int fd) const {
  std::string response = status;
  for (std::map<std::string, std::string>::const_iterator it = fields.begin();
       it != fields.end(); ++it) {
    response += it->first + ": " + it->second + "\r\n";
  }
  response += "\r\n" + _body;
  int write_result = write(fd, response.c_str(), response.size());
  if (close(fd) == -1) throw std::runtime_error("response: close error");
  if (write_result < 0) throw std::runtime_error("response: write error");
}