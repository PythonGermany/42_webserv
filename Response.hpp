#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <fcntl.h>
#include <unistd.h>

#include <map>
#include <stdexcept>
#include <string>

class Response {
 private:
  std::string status;
  std::map<std::string, std::string> fields;
  std::string _body;

 public:
  Response();
  Response(std::string status_code, std::string message);
  Response(const Response& other);
  Response& operator=(const Response& other);
  ~Response();

  void set_field(std::string key, std::string value);
  void setBody(std::string body);
  void send(int fd) const;
};

#endif