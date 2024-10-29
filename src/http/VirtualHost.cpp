#include "VirtualHost.hpp"

#include "utils.hpp"
#include "webserv.hpp"

std::vector<VirtualHost> VirtualHost::_virtualHosts;

std::map<std::string, std::string> VirtualHost::_mimeTypes;

VirtualHost::VirtualHost() : _externalServerId(WEBSERV_NAME) {}

VirtualHost::VirtualHost(const Context &context) {
  _context = context;
  setExternalServerId();

  std::vector<std::vector<std::string> > &listens =
      _context.getDirective("listen");
  for (size_t i = 0; i < listens.size(); i++)
    addSets(_resolvedListenDirective, Address::resolveHost(listens[i][0]));
}

VirtualHost::VirtualHost(const VirtualHost &rhs) { *this = rhs; }

VirtualHost &VirtualHost::operator=(const VirtualHost &rhs) {
  if (this == &rhs) return *this;
  _context = rhs._context;
  _externalServerId = rhs._externalServerId;
  _resolvedListenDirective = rhs._resolvedListenDirective;
  return *this;
}

VirtualHost::~VirtualHost() {}

void VirtualHost::add(const VirtualHost &virtualHost) {
  _virtualHosts.push_back(virtualHost);
}

void VirtualHost::setMimeTypes(std::map<std::string, std::string> &mimeTypes) {
  _mimeTypes = mimeTypes;
}

std::vector<VirtualHost> &VirtualHost::getVirtualHosts() {
  return _virtualHosts;
}

std::string VirtualHost::getMimeType(std::string extension) {
  std::map<std::string, std::string>::iterator it = _mimeTypes.find(extension);
  if (it != _mimeTypes.end()) return it->second;
  return "";
}
std::string const &VirtualHost::getAddress() {
  return _context.getDirective("listen")[0][0];
}

std::set<Address> const &VirtualHost::getResolvedAddress() const {
  return _resolvedListenDirective;
}

Context &VirtualHost::getContext() { return _context; }

const std::string &VirtualHost::getExternalServerId() const {
  return _externalServerId;
}

VirtualHost *VirtualHost::matchVirtualHost(Address &address, std::string host) {
  std::vector<VirtualHost *> possibleHosts;

  for (std::vector<VirtualHost>::iterator it = _virtualHosts.begin();
       it != _virtualHosts.end(); ++it) {
    std::set<Address> const &listenDirective = it->getResolvedAddress();
    for (std::set<Address>::const_iterator itLd = listenDirective.begin();
         itLd != listenDirective.end(); ++itLd) {
      if (*itLd == address)
        possibleHosts.push_back(&(*it));
      else if (*itLd == Address(address.family(), address.port()))
        possibleHosts.push_back(&(*it));
    }
  }
  for (std::vector<VirtualHost *>::iterator it = possibleHosts.begin();
       it != possibleHosts.end(); ++it) {
    if ((*it)->getContext().exists("server_name")) {
      std::vector<std::string> &serverNames =
          (*it)->getContext().getDirective("server_name")[0];
      for (std::vector<std::string>::const_iterator itSn = serverNames.begin();
           itSn != serverNames.end(); ++itSn) {
        int diff = std::strcmp(itSn->c_str(), host.c_str());
        if (!diff || diff == ':' || -diff == ':') return *it;
      }
    }
  }
  if (possibleHosts.empty()) return NULL;
  return *possibleHosts.begin();
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

void VirtualHost::setExternalServerId() {
  if (_context.exists("server_tokens", true) &&
      _context.getDirective("server_tokens", true)[0][0] == "on")
    _externalServerId = WEBSERV_ID;
  else
    _externalServerId = WEBSERV_NAME;
}
