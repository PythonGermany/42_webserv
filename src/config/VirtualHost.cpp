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

std::string VirtualHost::getMimeType(std::string extension) {
  std::map<std::string, std::string>::iterator it = _mimeTypes.find(extension);
  if (it != _mimeTypes.end()) return it->second;
  return "";
}

Context &VirtualHost::getContext() { return _context; }

Context *VirtualHost::matchLocation(const std::string &uri) {
  std::vector<Context> &locations = _context.getContext("location");
  Context *match = NULL;
  size_t matchSize = 0;
  for (size_t i = 0; i < locations.size(); i++) {
    std::string locUri = locations[i].getDirective("url")[0];
    if (startsWith(uri, locUri) && locUri.size() > matchSize) {
      match = &locations[i];
      matchSize = locUri.size();
    }
  }
  return match;
}

void VirtualHost::print() { _context.print(); }