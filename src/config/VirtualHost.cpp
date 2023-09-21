#include "VirtualHost.hpp"

std::vector<VirtualHost> VirtualHost::_virtualHosts;

std::map<std::string, std::string> VirtualHost::_mimeTypes;

VirtualHost::VirtualHost() {}

VirtualHost::VirtualHost(const Context &context) {
  _context = context;
  std::string listen = _context.getDirective("listen")[0];
  bool ipv6 = startsWith(listen, "[");
  std::string address, port;
  if (ipv6) {
    size_t pos = listen.find("]:");
    if (pos == std::string::npos)
      throw std::runtime_error("Invalid listen directive");
    address = listen.substr(1, pos - 1);
    port = listen.substr(pos + 2);
  } else {
    size_t pos = listen.find(":");
    if (pos == std::string::npos)
      throw std::runtime_error("Invalid listen directive");
    address = listen.substr(0, pos);
    port = listen.substr(pos + 1);
  }
  _address = Address(address, port);
  _context.removeDirective("listen");
}

VirtualHost::VirtualHost(const VirtualHost &rhs) { *this = rhs; }

VirtualHost &VirtualHost::operator=(const VirtualHost &rhs) {
  if (this == &rhs) return *this;
  _address = rhs._address;
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

Address &VirtualHost::getAddress() { return _address; }

Context &VirtualHost::getContext() { return _context; }

VirtualHost *VirtualHost::matchVirtualHost(Address &address,
                                           std::string serverName) {
  VirtualHost *match = NULL;
  for (size_t i = 0; i < _virtualHosts.size(); i++) {
    if (_virtualHosts[i].getAddress() == address)
      if (match == NULL) match = &_virtualHosts[i];
    std::vector<std::string> &names =
        _virtualHosts[i].getContext().getDirective("server_name");
    for (size_t j = 0; j < names.size(); j++)
      if (names[j] == serverName) return &_virtualHosts[i];
  }
  return match;
}

Context *VirtualHost::matchLocation(const std::string &uri) {
  std::vector<Context> &locations = _context.getContext("location");
  Context *match = &_context;
  size_t matchSize = 0;
  for (size_t i = 0; i < locations.size(); i++) {
    std::string locUri = locations[i].getDirective("url")[0];
    if (!endsWith(locUri, "/")) locUri += "/";
    if (locUri == uri) return &locations[i];
    if (startsWith(uri, locUri) && locUri.size() > matchSize) {
      match = &locations[i];
      matchSize = locUri.size();
    }
  }
  return match;
}

void VirtualHost::print() { _context.print(); }