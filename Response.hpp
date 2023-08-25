#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <unistd.h>

#include <map>
#include <stdexcept>
#include <string>

class Response {
 private:
  std::string status;
  std::map<std::string, std::string> fields;
  std::string body;

 public:
  Response();
  Response(std::string status_code, std::string message);
  Response(const Response& other);
  Response& operator=(const Response& other);
  ~Response();

  void set_field(std::string key, std::string value);
  void set_body(int fd);
  void set_body(std::string body);
  std::string get() const;
};

#endif