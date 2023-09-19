#include "Response.hpp"

Response::Response() {}

Response::Response(std::string version, std::string statusCode,
                   std::string reasonPhrase)
    : _version(version), _statusCode(statusCode), _reasonPhrase(reasonPhrase) {}

Response::Response(const Response &rhs) { *this = rhs; }

Response &Response::operator=(const Response &rhs) {
  if (this == &rhs) return *this;
  _version = rhs._version;
  _statusCode = rhs._statusCode;
  _reasonPhrase = rhs._reasonPhrase;
  _headers = rhs._headers;
  _body = rhs._body;
  return *this;
}

Response::~Response() {}

void Response::setVersion(std::string version) { _version = version; }

void Response::setStatusCode(std::string statusCode) {
  _statusCode = statusCode;
}

void Response::setReasonPhrase(std::string reasonPhrase) {
  _reasonPhrase = reasonPhrase;
}

void Response::setHeaders(std::map<std::string, std::string> &headers) {
  _headers = headers;
}

void Response::setBody(std::string body) { _body = body; }

void Response::setHeader(std::string key, std::string value) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  _headers[key] = value;
}

std::string Response::getHeader(std::string key) const {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  std::map<std::string, std::string>::const_iterator it = _headers.find(key);
  if (it == _headers.end()) return "";
  return it->second;
}

std::string Response::getBody() const { return _body; }

std::string Response::generate() {
  std::string response =
      _version + " " + _statusCode + " " + _reasonPhrase + "\r\n";
  for (std::map<std::string, std::string>::iterator it = _headers.begin();
       it != _headers.end(); ++it) {
    response += it->first + ": " + it->second + "\r\n";
  }
  response += "\r\n" + _body;
  return response;
}