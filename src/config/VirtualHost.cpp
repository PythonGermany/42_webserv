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

VirtualHost *VirtualHost::matchVirtualHost(Address &address, std::string host) {
  VirtualHost *match = NULL;
  for (size_t i = 0; i < _virtualHosts.size(); i++) {
    // Check if address matches
    if (_virtualHosts[i].getAddress() == address) {
      // Set default match if none was found already
      if (match == NULL) match = &_virtualHosts[i];

      // Check if server name matches
      if (_virtualHosts[i].getContext().exists("server_name")) {
        std::vector<std::string> &names =
            _virtualHosts[i].getContext().getDirective("server_name");
        for (size_t j = 0; j < names.size(); j++)
          if (names[j] == host) return &_virtualHosts[i];
      }
    }
  }
  return match;
}

Context *VirtualHost::matchLocation(const std::string &uri) {
  Context *match = &_context;

  // Check if context exists
  if (_context.exists("location")) {
    std::vector<Context> &locations = _context.getContext("location");
    size_t matchSize = 0;
    for (size_t i = 0; i < locations.size(); i++) {
      std::string locUri = locations[i].getArgs()[0];

      // Add trailing slash if missing to avoid matching /test with /test2
      if (!endsWith(locUri, "/")) locUri += "/";

      // Check if location matches
      if (locUri == uri) return &locations[i];
      if (startsWith(uri, locUri) && locUri.size() > matchSize) {
        match = &locations[i];
        matchSize = locUri.size();
      }
    }
  }
  return match;
}

void VirtualHost::print() { _context.print(); }