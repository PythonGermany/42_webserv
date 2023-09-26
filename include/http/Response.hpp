#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <algorithm>
#include <map>
#include <string>

class Response {
 private:
  std::string _version;
  std::string _status;
  std::string _reason;
  std::map<std::string, std::string> _headers;
  std::istream *_body;

 public:
  Response();
  Response(std::string version, std::string status, std::string reason);
  Response(const Response &rhs);
  Response &operator=(const Response &rhs);
  ~Response();

  void setVersion(std::string version);
  void setStatus(std::string status);
  void setReason(std::string reason);
  void setHeaders(std::map<std::string, std::string> &headers);
  void setBody(std::istream *body);

  void setHeader(std::string key, std::string value);

  std::string getVersion() const;
  std::string getStatus() const;
  std::string getReason() const;
  std::string getHeader(std::string key) const;

  std::istream *getBody();

  std::string generateHead();
};

#endif