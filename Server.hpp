#ifndef SERVER_HPP
#define SERVER_HPP

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "Socket.hpp"

struct location {
  std::string _path;
  std::vector<std::string> _methods;
  std::string _redirect;
  std::string _root;
  std::string _index;
  bool _autoindex;
  std::string _cgi_path;
  std::string _cgi_extension;
};

class Server {
 private:
  Socket *_socket;
  std::string _host;
  int _port;
  std::vector<std::string> _names;
  std::map<std::string, std::string> _error_pages;
  int _client_max_body_size;
  std::vector<location> _locations;

 public:
  Server();
  // Server(std::string config);
  ~Server();

  void set_socket(Socket *socket);
  void set_host(std::string host);
  void set_port(int port);
  void set_names(std::vector<std::string> names);
  void set_error_pages(std::map<std::string, std::string> error_pages);
  void set_client_max_body_size(int client_max_body_size);
  void set_locations(std::vector<location> locations);

  std::map<std::string, std::string> &error_pages() const;

  location resolve_location(std::string path);
};

#endif