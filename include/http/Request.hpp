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

  bool _error;

 public:
  Request();
  Request(const Request &rhs);
  Request &operator=(const Request &rhs);
  ~Request();

  void setHeader(std::string key, std::string value);

  std::string getMethod() const;
  Uri &getUri();
  std::string getVersion() const;
  std::string getHeader(std::string key) const;

  void deleteHeaderField(std::string key, std::string value);

  void parseHead(std::string msg);
  void parseHeader(std::string fields);
  bool isValid() const;
};

#endif