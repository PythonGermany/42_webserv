#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <algorithm>
#include <istream>
#include <list>
#include <map>
#include <string>

class Response {
 private:
  std::string _version;
  std::string _status;
  std::string _reason;
  std::map<std::string, std::string> _headers;
  std::list<std::string> _cookies;
  std::istream *_body;

  bool _ready;

 public:
  Response();
  Response(std::string version, std::string status, std::string reason);
  Response &operator=(const Response &rhs);
  ~Response();

  void init(std::string version, std::string status, std::string reason);
  void clear();

  void setVersion(std::string version);
  void setStatus(std::string status);
  void setReason(std::string reason);
  void setHeaders(std::map<std::string, std::string> &headers);
  void setCookie(const std::string &cookie);
  void setBody(std::istream *body);
  void setReady(bool ready = true);

  void setHeader(std::string key, std::string value);

  std::string getVersion() const;
  std::string getStatus() const;
  std::string getReason() const;
  std::string getHeader(std::string key) const;

  std::istream *getBody();
  std::istream *resetBody();

  bool isReady();

  std::string generateHead();
};

#endif