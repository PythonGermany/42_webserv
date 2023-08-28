#include "Server.hpp"

Server::Server() {
  _host = "0.0.0.0";
  _port = "";
  _client_max_body_size = 1024 * 1024;
  _isDefault = false;
}

Server::~Server() {}

void Server::setHost(std::string host) { _host = host; }

void Server::setPort(std::string port) { _port = port; }

void Server::addName(std::string name) { _names.push_back(name); }

void Server::setNames(std::vector<std::string> names) { _names = names; }

void Server::addErrorPage(std::string code, std::string path) {
  _error_pages[code] = path;
}

void Server::setClientMaxBodySize(int size) { _client_max_body_size = size; }

void Server::addLocation(location location) { _locations.push_back(location); }

void Server::setIsDefault(bool isDefault) { _isDefault = isDefault; }

std::string Server::getHost() { return _host; }

std::string Server::getPort() { return _port; }

std::vector<std::string> Server::getNames() { return _names; }

std::map<std::string, std::string> Server::getErrorPages() {
  return _error_pages;
}

int Server::getClientMaxBodySize() { return _client_max_body_size; }

bool Server::getIsDefault() { return _isDefault; }

location Server::matchLocation(std::string path) {
  location *match = &_locations[0];
  for (size_t i = 1; i < _locations.size(); i++) {
    if (path.find(_locations[i].path) == 0 &&
        _locations[i].path.length() > (*match).path.length()) {
      match = &_locations[i];
    }
  }
  if (match == NULL) throw std::runtime_error("No matching location found");
  return *match;
}

std::vector<location> Server::getLocations() { return _locations; }

void Server::print() {
  std::cout << "Server:" << std::endl;
  std::cout << "  host: " << _host << std::endl;
  std::cout << "  port: " << _port << std::endl;
  for (std::vector<std::string>::iterator it = _names.begin();
       it != _names.end(); it++) {
    std::cout << "  name: " << *it << std::endl;
  }
  for (std::map<std::string, std::string>::iterator it = _error_pages.begin();
       it != _error_pages.end(); it++) {
    std::cout << "  error_page: " << it->first << " " << it->second << " "
              << std::endl;
  }
  std::cout << "  client_max_body_size: " << _client_max_body_size << std::endl;
  for (std::vector<location>::iterator it = _locations.begin();
       it != _locations.end(); it++) {
    std::cout << "  location: " << std::endl;
    std::cout << "    path: " << it->path << std::endl;
    for (std::vector<std::string>::iterator it2 = it->methods.begin();
         it2 != it->methods.end(); it2++)
      std::cout << "    method: " << *it2 << std::endl;
    std::cout << "    redirect: " << it->redirect << std::endl;
    std::cout << "    root: " << it->root << std::endl;
    for (std::vector<std::string>::iterator it2 = it->_index.begin();
         it2 != it->_index.end(); it2++) {
      std::cout << "    index: " << *it2 << std::endl;
    }
    std::cout << "    autoindex: " << it->_autoindex << std::endl;
    for (std::map<std::string, std::string>::iterator it2 = it->cgi.begin();
         it2 != it->cgi.end(); it2++) {
      std::cout << "    cgi: " << it2->first << " " << it2->second << std::endl;
    }
  }
  std::cout << "  isDefault: " << _isDefault << std::endl;
  std::cout << "----------------------------------------------------"
            << std::endl;
}