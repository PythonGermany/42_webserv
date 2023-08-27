#include "Server.hpp"

Server::Server() {}

Server::~Server() {}

void Server::setSocket(Socket *socket) { _socket = socket; }

void Server::setHost(std::string host) { _host = host; }

void Server::setPort(int port) { _port = port; }

void Server::addName(std::string name) { _names.push_back(name); }

void Server::addErrorPage(std::string code, std::string path) {
  _error_pages[code] = path;
}

void Server::setClientMaxBodySize(int size) { _client_max_body_size = size; }

void Server::addLocation(location location) { _locations.push_back(location); }

location Server::resolveLocation(std::string path) {
  // Go over all location paths and find the longest match
  location longest_match;
  for (std::vector<location>::iterator it = _locations.begin();
       it != _locations.end(); it++) {
    if (path.find(it->_path) == 0 &&
        it->_path.length() > longest_match._path.length()) {
      longest_match = *it;
    }
  }
  return longest_match;
}