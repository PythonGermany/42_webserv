#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "Uri.hpp"
#include "utils.hpp"

class Request {
 private:
  std::string _method;
  Uri _uri;
  std::string _version;
  std::map<std::string, std::string> _headers;
  std::string _body;

  bool _error;

 public:
  Request();
  Request(const Request &rhs);
  Request &operator=(const Request &rhs);
  ~Request();

  void setBody(std::string body);

  std::string getMethod() const;
  Uri getUri() const;
  std::string getVersion() const;
  std::string getHeader(std::string key) const;
  std::string getBody() const;

  void parseHead(std::string msg);
  bool isValid() const;
};

#endif