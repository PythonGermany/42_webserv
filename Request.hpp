#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <unistd.h>

#include <map>
#include <stdexcept>
#include <string>

class Request {
 private:
  std::string method;
  std::string uri;
  std::string version;
  std::map<std::string, std::string> fields;
  std::string body;

 public:
  Request();
  Request(int fd);
  ~Request();

  std::string get_method(void) const;
  std::string get_uri(void) const;
  std::string get_version(void) const;
  std::string get_field(std::string key) const;
  std::string get_body(void) const;
};

#endif