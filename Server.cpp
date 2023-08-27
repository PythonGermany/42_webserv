#include "Server.hpp"

Server::Server() {}

Server::~Server() {}

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