#include "Uri.hpp"

Uri::Uri() : _path("/") {}

Uri::Uri(std::string uri) : _path("/") { load(uri); }

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

  // Find scheme end
  pos = uri.find("://");
  if (pos != std::string::npos) {
    _scheme = uri.substr(0, pos);
    uri = uri.substr(pos + 3);
  }

  // Find host end
  pos = uri.find(":");
  if (pos != std::string::npos) {
    _host = uri.substr(0, pos);
    uri = uri.substr(pos + 1);

    // Find port end
    pos = uri.find("/");
    if (pos != std::string::npos) {
      _port = uri.substr(0, pos);
      uri = uri.substr(pos);
    } else {
      _port = uri;
      uri = "";
    }
  } else {
    // Find host end
    pos = uri.find("/");
    if (pos != std::string::npos) {
      _host = uri.substr(0, pos);
      uri = uri.substr(pos);
    } else {
      _host = uri;
      uri = "";
    }
  }
  // Find query start
  pos = uri.find("?");
  if (pos != std::string::npos) {
    _path = uri.substr(0, pos);
    _query = uri.substr(pos + 1);
  } else
    _path = uri;

  // Transform required fields to lowercase
  std::transform(_scheme.begin(), _scheme.end(), _scheme.begin(), ::tolower);
  std::transform(_host.begin(), _host.end(), _host.begin(), ::tolower);
}

void Uri::setScheme(std::string scheme) { _scheme = scheme; }

void Uri::setHost(std::string host) { _host = host; }

void Uri::setPort(std::string port) { _port = port; }

void Uri::setPath(std::string path) { _path = path; }

void Uri::setQuery(std::string query) { _query = query; }

std::string Uri::getScheme() const { return _scheme; }

std::string Uri::getHost() const { return _host; }

std::string Uri::getPort() const { return _port; }

std::string Uri::getPath() const { return _path; }

std::string Uri::getQuery() const { return _query; }

size_t Uri::decode() {
  try {
    _scheme = percentDecode(_scheme);
    _host = percentDecode(_host);
    _port = percentDecode(_port);
    _path = percentDecode(_path);
    _query = percentDecode(_query);
  } catch (const std::exception &e) {
    return 1;
  }
  return 0;
}

std::string Uri::encode() const {
  std::string uri;
  if (_scheme != "" && _host != "") uri += percentEncode(_scheme, "") + "://";
  uri += percentEncode(_host, "@");
  if (_port != "" && _port != "80") uri += ":" + percentEncode(_port, "");
  uri += percentEncode(_path, "/.");
  if (_query != "") uri += "?" + percentEncode(_query, "?=&");
  return uri;
}

std::string Uri::generate() const {
  std::string uri;
  if (_scheme != "" && _host != "") uri += _scheme + "://";
  uri += _host;
  if (_port != "" && _port != "80") uri += ":" + _port;
  uri += _path;
  if (_query != "") uri += "?" + _query;
  return uri;
}

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