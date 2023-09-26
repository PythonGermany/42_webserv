#include "Request.hpp"

Request::Request() { _error = false; }

Request::Request(const Request &rhs) { *this = rhs; }

Request &Request::operator=(const Request &rhs) {
  if (this == &rhs) return *this;
  _method = rhs._method;
  _uri = rhs._uri;
  _version = rhs._version;
  _headers = rhs._headers;
  _error = rhs._error;
  return *this;
}

Request::~Request() {}

void Request::setHeader(std::string key, std::string value) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  trimStart(value, " \t");
  if (_headers.find(key) != _headers.end())
    _headers[key] += ", " + value;
  else
    _headers[key] = value;
}

std::string Request::getMethod() const { return _method; }

Uri &Request::getUri() { return _uri; }

std::string Request::getVersion() const { return _version; }

std::string Request::getHeader(std::string key) const {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  std::map<std::string, std::string>::const_iterator it = _headers.find(key);
  if (it == _headers.end()) return "";
  return it->second;
}

void Request::deleteHeaderField(std::string key, std::string value) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  std::map<std::string, std::string>::iterator it = _headers.find(key);
  if (it == _headers.end()) return;
  std::string &field = it->second;
  size_t pos = field.find(value);
  if (pos == std::string::npos) return;
  field.erase(pos, value.size());
  trim(field, ", ");
  if (field.empty()) _headers.erase(it);
}

void Request::parseHead(std::string msg) {
  // Find head delimiter
  size_t pos = msg.find("\r\n");
  if (pos == std::string::npos) {
    _error = true;
    return;
  }

  // Parse request line
  std::string requestLine = msg.substr(0, pos);
  std::vector<std::string> requestLineTokens = split(requestLine, " ");
  if (requestLineTokens.size() != 3) {
    _error = true;
    return;
  }
  _method = requestLineTokens[0];
  _uri = Uri(requestLineTokens[1]);
  _version = requestLineTokens[2];

  // Parse header
  parseHeader(msg.substr(pos + 2));
}

void Request::parseHeader(std::string fields) {
  size_t end = fields.find("\r\n");

  while (end != std::string::npos) {
    std::string line = fields.substr(0, end);
    if (line.empty()) break;
    size_t colon = line.find(":");
    if (colon == std::string::npos) {
      _error = true;
      return;
    }
    std::string key = line.substr(0, colon);
    std::string value = line.substr(colon + 1);
    setHeader(key, value);
    fields.erase(0, line.size() + 2);
    end = fields.find("\r\n");
  }
}

bool Request::isValid() const {
  if (_error) return false;
  if (_uri.pathOutOfBound() || !startsWith(_version, "HTTP/")) return false;

  // Host header is always required
  // https://datatracker.ietf.org/doc/html/rfc2616#section-9
  if (_headers.find("host") == _headers.end()) return false;

  return true;
}