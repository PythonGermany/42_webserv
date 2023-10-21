#include "Request.hpp"

Request::Request() {}

Request::Request(const Request &rhs) { *this = rhs; }

Request &Request::operator=(const Request &rhs) {
  if (this == &rhs) return *this;
  _method = rhs._method;
  _uri = rhs._uri;
  _version = rhs._version;
  _headers = rhs._headers;
  return *this;
}

Request::~Request() {}

void Request::setHeaderField(std::string key, const std::string &value) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  _headers[key].clear();
  std::list<std::string> values = split<std::list<std::string> >(value, ",");
  for (std::list<std::string>::iterator it = values.begin(); it != values.end();
       ++it)
    setHeaderFieldValue(key, *it);
}

void Request::setHeaderFieldValue(std::string key, const std::string &value) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  _headers[key].push_back(trim(value, WHITESPACE));
}

std::string Request::getMethod() const { return _method; }

Uri &Request::getUri() { return _uri; }

std::string Request::getVersion() const { return _version; }

std::string Request::getHeaderField(std::string key) const {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  std::map<std::string, std::list<std::string> >::const_iterator it =
      _headers.find(key);
  if (it == _headers.end()) return "";
  return concatenate<std::list<std::string> >(it->second, ", ");
}

void Request::removeHeader(std::string key) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  _headers.erase(key);
}

void Request::removeHeaderValue(std::string key, const std::string &value) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  if (_headers.find(key) != _headers.end()) {
    std::list<std::string>::iterator it =
        std::find(_headers[key].begin(), _headers[key].end(), value);
    if (it != _headers[key].end()) _headers[key].erase(it);
  }
}

bool Request::isMethod(const std::string &method) { return _method == method; }

bool Request::hasHeaderFieldValue(std::string key, const std::string &value) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  std::map<std::string, std::list<std::string> >::const_iterator it =
      _headers.find(key);
  if (it == _headers.end()) return false;
  return std::find(it->second.begin(), it->second.end(), value) !=
         it->second.end();
}

int Request::parseRequestLine(std::string line) {
  std::vector<std::string> requestLineTokens =
      split<std::vector<std::string> >(line, " ", true);
  if (requestLineTokens.size() != 3) return 1;
  _method = requestLineTokens[0];
  _version = requestLineTokens[2];

  // https://datatracker.ietf.org/doc/html/rfc9112#name-method and
  // https://datatracker.ietf.org/doc/html/rfc9112#name-http-version
  if (_method.find_first_of(WHITESPACE) != std::string::npos ||
      _version.find_first_of(WHITESPACE) != std::string::npos)
    return 1;
  if (!startsWith(_version, "HTTP/")) return 1;
  return _uri.load(requestLineTokens[1]);
}

int Request::parseHeaderFields(std::string fields) {
  size_t end;
  while (true) {
    end = fields.find("\r\n");
    if (end == std::string::npos) end = fields.size();
    std::string line = fields.substr(0, end);
    size_t colon = line.find(":");
    if (colon == std::string::npos) return 1;

    std::string key = line.substr(0, colon);
    if (key.find_first_of(WHITESPACE) != std::string::npos) return 1;

    std::string value = line.substr(colon + 1);
    setHeaderField(key, value);
    fields.erase(0, line.size() + 2);
    if (fields.empty()) return 0;
  }
}