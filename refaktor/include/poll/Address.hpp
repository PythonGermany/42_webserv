#ifndef ADDRESS_HPP
#define ADDRESS_HPP

#include <arpa/inet.h>  //inet_*
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>  //AF_INET

#include <cerrno>
#include <cstdlib>  //atoi
#include <cstring>
#include <limits>
#include <ostream>  //overload
#include <sstream>
#include <stdexcept>
#include <string>

#include <set>
#include <utility>

/**
 * @brief stores either an ipv4 or an ipv6 address with port
 * @throw std::runtime_error() can be thrown by Constructor and stream insertion
 * operator
 */
class Address {
 public:
  Address();
  Address(sa_family_t family, in_port_t port);
  Address(sa_family_t family, sockaddr *src);
  Address(Address const &other);
  Address &operator=(Address const &other);
  ~Address();
  sa_family_t family() const;
  void family(sa_family_t family);
  void const *addr() const;
  sockaddr *data();
  sockaddr const *data() const;
  socklen_t size() const;
  void size(socklen_t size);
  void port(in_port_t port);
  in_port_t port() const;

  bool operator<(Address const &other) const;
  static std::set<Address> resolveHost(std::string const &src);
  bool operator==(Address const &other) const;

 private:
  union _Address {
    struct sockaddr_in inet;
    struct sockaddr_in6 inet6;
  };
  _Address _addr;
  sa_family_t _family;
};

std::ostream &operator<<(std::ostream &os, Address const &addr);

#endif  // ADDRESS_HPP
