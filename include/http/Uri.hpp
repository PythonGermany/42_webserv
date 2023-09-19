#ifndef URI_HPP
#define URI_HPP

#include <string>

#include "utils.hpp"

class Uri {
 private:
  std::string _scheme;
  std::string _host;
  std::string _port;
  std::string _path;
  std::string _query;

 public:
  Uri();
  Uri(std::string uri);
  Uri(const Uri &rhs);
  Uri &operator=(const Uri &rhs);
  ~Uri();

  std::string getScheme() const;
  std::string getHost() const;
  std::string getPort() const;
  std::string getPath() const;
  std::string getQuery() const;

  bool pathOutOfBound() const;
};

#endif