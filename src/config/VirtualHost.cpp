#include "VirtualHost.hpp"

std::vector<VirtualHost> VirtualHost::_virtualHosts;

std::map<std::string, std::string> VirtualHost::_mimeTypes;

VirtualHost::VirtualHost() {}

VirtualHost::VirtualHost(const Context &context) { _context = context; }

VirtualHost::VirtualHost(const VirtualHost &rhs) { *this = rhs; }

VirtualHost &VirtualHost::operator=(const VirtualHost &rhs) {
  if (this == &rhs) return *this;
  _context = rhs._context;
  return *this;
}

VirtualHost::~VirtualHost() {}

void VirtualHost::add(const VirtualHost &virtualHost) {
  _virtualHosts.push_back(virtualHost);
}

void VirtualHost::setMimeTypes(std::map<std::string, std::string> &mimeTypes) {
  _mimeTypes = mimeTypes;
}

void VirtualHost::setContext(const Context &context) { _context = context; }

std::vector<VirtualHost> &VirtualHost::getVirtualHosts() {
  return _virtualHosts;
}

Context &VirtualHost::getContext() { return _context; }

void VirtualHost::print() { _context.print(); }