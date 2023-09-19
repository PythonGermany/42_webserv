#include "Uri.hpp"

Uri::Uri() : _scheme("http"), _host(""), _port("80"), _path("/"), _query("") {}

Uri::Uri(std::string uri)
    : _scheme("http"), _host(""), _port("80"), _path("/"), _query("") {
  load(uri);
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

void Uri::load(std::string uri) {
  size_t pos;
  pos = uri.find("://");
  if (pos != std::string::npos) {
    _scheme = uri.substr(0, pos);
    uri = uri.substr(pos + 3);
  }
  pos = uri.find(":");
  if (pos != std::string::npos) {
    _host = uri.substr(0, pos);
    uri = uri.substr(pos + 1);
    pos = uri.find("/");
    if (pos != std::string::npos) {
      _port = uri.substr(0, pos);
      uri = uri.substr(pos);
    } else {
      _port = uri;
      uri = "";
    }
  } else {
    pos = uri.find("/");
    if (pos != std::string::npos) {
      _host = uri.substr(0, pos);
      uri = uri.substr(pos);
    } else {
      _host = uri;
      uri = "";
    }
  }
  pos = uri.find("?");
  if (pos != std::string::npos) {
    _path = uri.substr(0, pos);
    _query = uri.substr(pos + 1);
  } else
    _path = uri;
  std::transform(_scheme.begin(), _scheme.end(), _scheme.begin(), ::tolower);
  std::transform(_host.begin(), _host.end(), _host.begin(), ::tolower);
}

std::string Uri::getScheme() const { return _scheme; }

std::string Uri::getHost() const { return _host; }

std::string Uri::getPort() const { return _port; }

std::string Uri::getPath() const { return _path; }

std::string Uri::getQuery() const { return _query; }

bool Uri::compare(const Uri &rhs) const {
  return _scheme == rhs._scheme && _host == rhs._host && _port == rhs._port &&
         _path == rhs._path && _query == rhs._query;
}

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