#include "Server.hpp"

Server::Server() {}

// Server::Server(std::string config) {}

Server::~Server() {}

void Server::set_socket(Socket *socket) { _socket = socket; }

void Server::set_host(std::string host) { _host = host; }

void Server::set_port(int port) { _port = port; }

void Server::set_names(std::vector<std::string> names) { _names = names; }

void Server::set_error_pages(std::map<std::string, std::string> error_pages) {
  _error_pages = error_pages;
}

void Server::set_client_max_body_size(int client_max_body_size) {
  _client_max_body_size = client_max_body_size;
}

void Server::set_locations(std::vector<location> locations) {
  _locations = locations;
}

std::map<std::string, std::string> &Server::error_pages() const {
  return const_cast<std::map<std::string, std::string> &>(_error_pages);
}

location Server::resolve_location(std::string path) {
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