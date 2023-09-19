#include "Uri.hpp"

Uri::Uri() {}

Uri::Uri(std::string uri) {
  size_t pos = uri.find("://");
  if (pos == std::string::npos) {
    _scheme = "http";
    pos = 0;
  } else {
    _scheme = uri.substr(0, pos);
    pos += 3;
  }
  size_t pos2 = uri.find(":", pos);
  if (pos2 == std::string::npos) {
    pos2 = uri.find("/", pos);
    if (pos2 == std::string::npos) {
      _host = uri.substr(pos);
      _port = "80";
      _path = "/";
      _query = "";
      return;
    }
    _host = uri.substr(pos, pos2 - pos);
    _port = "80";
    pos = pos2;
  } else {
    _host = uri.substr(pos, pos2 - pos);
    pos = pos2 + 1;
    pos2 = uri.find("/", pos);
    if (pos2 == std::string::npos) {
      _port = uri.substr(pos);
      _path = "/";
      _query = "";
      return;
    }
    _port = uri.substr(pos, pos2 - pos);
    pos = pos2;
  }
  pos2 = uri.find("?", pos);
  if (pos2 == std::string::npos) {
    _path = uri.substr(pos);
    _query = "";
    return;
  }
  _path = uri.substr(pos, pos2 - pos);
  _query = uri.substr(pos2 + 1);
}

Uri::Uri(const Uri &rhs) { *this = rhs; }

Uri &Uri::operator=(const Uri &rhs) {
  if (this == &rhs) return *this;
  _scheme = rhs._scheme;
  _host = rhs._host;
  _port = rhs._port;
  _path = rhs._path;
  _query = rhs._query;
  return *this;
}

Uri::~Uri() {}

std::string Uri::getScheme() const { return _scheme; }

std::string Uri::getHost() const { return _host; }

std::string Uri::getPort() const { return _port; }

std::string Uri::getPath() const { return _path; }

std::string Uri::getQuery() const { return _query; }

#include <iostream>
bool Uri::pathOutOfBound() const {
  std::string path = _path;
  int depth = 0;
  size_t pos;
  while (path.size() > 0) {
    pos = path.find("/");
    if (pos != 0) {
      if ((startsWith(path, "..") && path.size() == 2) ||
          startsWith(path, "../"))
        depth--;
      else
        depth++;
      if (depth < 0) return true;
    }
    if (pos == std::string::npos) return false;
    path = path.substr(pos + 1);
  }
  return false;
}