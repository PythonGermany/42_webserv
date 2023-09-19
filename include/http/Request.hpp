#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "utils.hpp"

class Request {
 private:
  std::string _method;
  std::string _uri;
  std::string _version;
  std::map<std::string, std::string> _headers;
  std::string _body;

 public:
  Request();
  Request(const Request &rhs);
  Request &operator=(const Request &rhs);
  ~Request();

  std::string getMethod() const;
  std::string getUri() const;
  std::string getVersion() const;
  std::map<std::string, std::string> getHeaders() const;
  std::string getHeader(std::string key) const;
  std::string getBody() const;

  void parseHead(std::string msg);
  bool isValid() const;
};

#endif