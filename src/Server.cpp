#include "Server.hpp"

Server::Server() {}

Server::Server(const Context &context) { _context = context; }

Server::Server(const Server &rhs) { *this = rhs; }

Server &Server::operator=(const Server &rhs) {
  if (this == &rhs) return *this;
  _context = rhs._context;
  _isDefault = rhs._isDefault;
  return *this;
}

Server::~Server() {}

void Server::setMimeTypes(std::map<std::string, std::string> &mimeTypes) {
  _mimeTypes = mimeTypes;
}

void Server::setContext(const Context &context) { _context = context; }

Context &Server::getContext() { return _context; }

void Server::print() { _context.print(); }