#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <map>
#include <string>

class Response {
 private:
  std::string _version;
  std::string _statusCode;
  std::string _reasonPhrase;
  std::map<std::string, std::string> _headers;
  std::string _body;

 public:
  Response();
  Response(std::string version, std::string statusCode,
           std::string reasonPhrase);
  Response(const Response &rhs);
  Response &operator=(const Response &rhs);
  ~Response();

  void setVersion(std::string version);
  void setStatusCode(std::string statusCode);
  void setReasonPhrase(std::string reasonPhrase);
  void setHeaders(std::map<std::string, std::string> &headers);
  void setBody(std::string body);

  void setHeader(std::string key, std::string value);

  std::string getBody() const;

  std::string generate();
};

#endif