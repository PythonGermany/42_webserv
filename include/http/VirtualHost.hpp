#ifndef VIRTUALHOST_HPP
#define VIRTUALHOST_HPP

#include <map>
#include <string>
#include <vector>

#include "Address.hpp"
#include "Context.hpp"

class VirtualHost {
 private:
  static std::vector<VirtualHost> _virtualHosts;
  static std::map<std::string, std::string> _mimeTypes;
  Context _context;
  std::string _externalServerId;
  std::set<Address> _resolvedListenDirective;

 public:
  VirtualHost();
  VirtualHost(const Context &context);
  VirtualHost(const VirtualHost &rhs);
  VirtualHost &operator=(const VirtualHost &rhs);
  ~VirtualHost();

  // Setters/Adders
  static void add(const VirtualHost &virtualHost);
  static void setMimeTypes(std::map<std::string, std::string> &mimeTypes);

  // Getters
  static std::vector<VirtualHost> &getVirtualHosts();
  static std::string getMimeType(std::string extension);
  std::string const &getAddress();
  std::set<Address> const &getResolvedAddress() const;
  Context &getContext();
  std::string const &getExternalServerId() const;

  static VirtualHost *matchVirtualHost(Address &address, std::string host);
  Context *matchLocation(const std::string &uri);

 private:
  void setExternalServerId();
};

#endif
