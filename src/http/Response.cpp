#include "Response.hpp"

Response::Response() {}

Response::Response(std::string version, std::string status, std::string reason)
    : _version(version), _status(status), _reason(reason) {}

Response::Response(const Response &rhs) { *this = rhs; }

Response &Response::operator=(const Response &rhs) {
  if (this == &rhs) return *this;
  _version = rhs._version;
  _status = rhs._status;
  _reason = rhs._reason;
  _headers = rhs._headers;
  _body = rhs._body;
  return *this;
}

Response::~Response() {}

void Response::setVersion(std::string version) { _version = version; }

void Response::setStatus(std::string status) { _status = status; }

void Response::setReason(std::string reason) { _reason = reason; }

void Response::setHeaders(std::map<std::string, std::string> &headers) {
  _headers = headers;
}

void Response::setBody(std::string body) { _body = body; }

void Response::setHeader(std::string key, std::string value) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  _headers[key] = value;
}

std::string Response::getVersion() const { return _version; }

std::string Response::getStatus() const { return _status; }

std::string Response::getReason() const { return _reason; }

std::string Response::getHeader(std::string key) const {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  std::map<std::string, std::string>::const_iterator it = _headers.find(key);
  if (it == _headers.end()) return "";
  return it->second;
}

std::string Response::getBody() const { return _body; }

std::string Response::generate() {
  std::string response = _version + " " + _status + " " + _reason + "\r\n";
  for (std::map<std::string, std::string>::iterator it = _headers.begin();
       it != _headers.end(); ++it) {
    response += it->first + ": " + it->second + "\r\n";
  }
  response += "\r\n" + _body;
  return response;
}