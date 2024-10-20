#include "Response.hpp"

#include <algorithm>

Response::Response() : _body(NULL), _ready(false) {}

Response::Response(std::string version, std::string status, std::string reason)
    : _version(version), _status(status), _reason(reason), _body(NULL) {}

Response::~Response() { delete _body; }

void Response::init(std::string version, std::string status,
                    std::string reason) {
  _version = version;
  _status = status;
  _reason = reason;
  clear();
}

void Response::clear() {
  _headers.clear();
  delete _body;
  _body = NULL;
  _ready = false;
}

void Response::setVersion(std::string version) { _version = version; }

void Response::setStatus(std::string status) { _status = status; }

void Response::setReason(std::string reason) { _reason = reason; }

void Response::setHeaders(std::map<std::string, std::string> &headers) {
  _headers = headers;
}

void Response::setCookie(const std::string &cookie) {
  _cookies.push_back(cookie);
}

void Response::setBody(std::istream *body) {
  delete _body;
  _body = body;
}

void Response::setReady(bool ready) { _ready = ready; }

void Response::setHeader(std::string key, std::string value, bool overwrite) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  if (_headers.find(key) != _headers.end() && overwrite == false)
    _headers[key] += ", " + value;
  else
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

std::istream *Response::getBody() { return _body; }

std::istream *Response::resetBody() {
  std::istream *tmp = _body;
  _body = NULL;
  return tmp;
}

bool Response::isReady() { return _ready; }

std::string Response::generateHead() {
  std::string response = _version + " " + _status + " " + _reason + "\r\n";
  for (std::map<std::string, std::string>::iterator it = _headers.begin();
       it != _headers.end(); ++it) {
    response += it->first + ": " + it->second + "\r\n";
  }
  for (std::list<std::string>::iterator it = _cookies.begin();
       it != _cookies.end(); ++it) {
    response += "set-cookie: " + *it + "\r\n";
  }
  response += "\r\n";
  return response;
}
