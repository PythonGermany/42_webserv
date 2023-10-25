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
  Uri(const Uri &rhs);
  Uri &operator=(const Uri &rhs);
  ~Uri();

  // Splits an uri into its sections
  // @return Non-zero on error
  int load(std::string uri);

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

  // Tries to percent decode the URI
  // @return 0 if successful, non-zero otherwise
  int decode();

  // Percent encodes an uri
  std::string encode() const;

  // Generates the uri without percent encoding
  std::string generate() const;
  int resolveDots();

  bool compare(const Uri &rhs) const;
};

#endif
