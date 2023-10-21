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
  std::map<std::string, std::list<std::string> > _headers;

 public:
  Request();
  Request(const Request &rhs);
  Request &operator=(const Request &rhs);
  ~Request();

  void setHeaderField(std::string key, const std::string &value);
  void setHeaderFieldValue(std::string key, const std::string &value);

  std::string getMethod() const;
  Uri &getUri();
  std::string getVersion() const;
  std::string getHeaderField(std::string key) const;

  void removeHeader(std::string key);
  void removeHeaderValue(std::string key, const std::string &value);

  bool isMethod(const std::string &method);
  bool hasHeaderFieldValue(std::string key, const std::string &value);

  int parseRequestLine(std::string line);
  int parseHeaderFields(std::string fields);
};

#endif