#include "Server.hpp"

Server::Server() {}

Server::Server(const Context &context) {
  _context = context;
  _isDefault = false;
}

Server::Server(const Server &rhs) { *this = rhs; }

Server &Server::operator=(const Server &rhs) {
  if (this == &rhs) return *this;
  _context = rhs._context;
  _isDefault = rhs._isDefault;
  return *this;
}

Server::~Server() {}

void Server::setMimeTypes(std::map<std::string, std::string> mimeTypes) {
  (void)mimeTypes;  // TODO: Remove this line
  //_mimeTypes = mimeTypes;
}

void Server::setContext(Context context) { _context = context; }

void Server::setIsDefault(bool isDefault) { _isDefault = isDefault; }

Context Server::getContext() { return _context; }

bool Server::getIsDefault() { return _isDefault; }

void Server::print() { _context.print(); }