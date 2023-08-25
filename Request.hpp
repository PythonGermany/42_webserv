#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <unistd.h>

#include <map>
#include <stdexcept>
#include <string>

class Request {
 private:
  std::string _method;
  std::string _uri;
  std::string _version;
  std::map<std::string, std::string> _fields;
  std::string _body;

 public:
  Request();
  Request(const Request& other);
  Request& operator=(const Request& other);
  Request(int fd);
  ~Request();

  std::string method(void) const;
  std::string uri(void) const;
  std::string version(void) const;
  std::string field(std::string key) const;
  std::string body(void) const;

 private:
  void throw_error(const std::string& error);
};

#endif