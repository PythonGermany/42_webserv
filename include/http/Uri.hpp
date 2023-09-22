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

  void load(std::string uri);

  void setScheme(std::string scheme);
  void setHost(std::string host);
  void setPort(std::string port);
  void setPath(std::string path);
  void setQuery(std::string query);

  std::string getScheme() const;
  std::string getHost() const;
  std::string getPort() const;
  std::string getPath() const;
  std::string getQuery() const;

  size_t decode();
  std::string encode();
  std::string generate() const;

  bool compare(const Uri &rhs) const;

  bool pathOutOfBound() const;
};

#endif