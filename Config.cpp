#include "Config.hpp"

Config::Config() {}

Config::Config(std::string path) { setFile(path); }

Config::Config(const Config &rhs) { *this = rhs; }

Config &Config::operator=(const Config &rhs) {
  if (this == &rhs) return *this;
  _file = rhs._file;
  return *this;
}

Config::~Config() {}

std::string Config::getFile() { return _file.Read(); }

void Config::setFile(std::string path) {
  _file = File(path);
  if (!_file.exists()) throwExeption("setFile", "File does not exist");
  if (!_file.file()) throwExeption("setFile", "Path is not a file");
  if (!_file.readable()) throwExeption("setFile", "File is not readable");
  _config = _file.Read();
}

std::vector<Server> Config::parseConfig() {
  std::vector<Server> servers;
  std::string data = trim(_config);

  while (data.length() > 0) {
    std::string token = trim(cut(data, 0, findToken(data, " ")));
    if (token == "server")
      servers.push_back(parseServer(cut(data, 0, findContextEnd(data))));
    else
      throwExeption("parse", "Unknown token '" + token + "'");
    data = trim(data);
  }
  return servers;
}

void Config::validateConfig(
    std::vector<Server>
        &servers) {  // TODO: complete, improve and test validation
  for (std::vector<Server>::iterator it = servers.begin(); it != servers.end();
       it++) {
    bool root_location = false;
    if (it->getHost() == "") throwExeption("validate", "Host not set");
    if (it->getPort() == "") throwExeption("validate", "Port not set");
    std::vector<location> locations = it->getLocations();
    for (std::vector<location>::iterator it2 = locations.begin();
         it2 != locations.end(); it2++) {
      if (it2->path == "") throwExeption("validate", "Location path not set");
      if (it2->path == "/") root_location = true;
      if (it2->methods.size() == 0)
        throwExeption("validate", "No methods set for location");
      if (it2->root == "" && it2->redirect == "")
        throwExeption("validate", "No root set for location");
      if (it2->_index.size() == 0 && it2->redirect == "")
        throwExeption("validate", "No index set for location");
    }
    if (!root_location)
      throwExeption("validate", "No root location set for server");
  }
}

void Config::setDefaultServers(std::vector<Server> &servers) {
  std::set<std::string> connection;
  for (std::vector<Server>::iterator it = servers.begin(); it != servers.end();
       it++) {
    std::string key = it->getHost() + ":" + it->getPort();
    if (connection.find(key) == connection.end()) {
      connection.insert(key);
      it->setIsDefault(true);
    }
  }
}

Server Config::parseServer(std::string context) {
  Server server;
  context = trimContext(context);
  while (context.length() > 0) {
    if (isContextBlock(context)) {
      parseContext(cut(context, 0, findContextEnd(context)), server);
    } else {
      std::string token = trim(cut(context, 0, findToken(context, " ")));
      std::string value = trim(cut(context, 0, findToken(context, ";")));
      if (token == "listen") {
        std::vector<std::string> listen = split(value, ":");
        if (listen.size() != 2)
          throwExeption("parseServer", "Expected 2 arguments for listen");
        server.setHost(listen[0]);
        if (!isNumeric(listen[1])) throwExeption("parseServer", "Invalid port");
        server.setPort(listen[1]);
      } else if (token == "server_name") {
        server.setNames(split(value, " "));
      } else if (token == "error_page") {
        std::vector<std::string> error_page = split(value, " ");
        if (error_page.size() != 2)
          throwExeption("parseServer", "Expected 2 arguments for error_page");
        server.addErrorPage(error_page[0], error_page[1]);
      } else if (token == "client_max_body_size") {
        if (!isNumeric(value))
          throwExeption("parseServer", "Invalid client_max_body_size");
        server.setClientMaxBodySize(std::atoi(value.c_str()));
        if (server.getClientMaxBodySize() == 0)
          throwExeption("parseServer", "Invalid client_max_body_size");
      } else
        throwExeption("parseServer", "Unknown token '" + token + "'");
    }
    context = trim(context.erase(0, 1));
  }
  return server;
}

void Config::parseContext(std::string context, Server &server) {
  std::string token = trim(cut(context, 0, findToken(context, " ")));
  if (token == "server") {
    throwExeption("parseContext", "Nested server context not allowed");
  } else if (token == "location") {
    server.addLocation(parseLocation(trim(context)));
  } else {
    throwExeption("parseContext", "Unknown context '" + context + "'");
  }
}

location Config::parseLocation(std::string context) {
  location location;
  location.path = trim(cut(context, 0, findToken(context, " ")));
  context = trimContext(context);
  while (context.length() > 0) {
    std::string token = trim(cut(context, 0, findToken(context, " ")));
    std::string value = trim(cut(context, 0, findToken(context, ";")));
    if (token == "methods") {
      location.methods = split(value, " ");
    } else if (token == "redirect") {
      location.redirect = value;
    } else if (token == "root") {
      if (!endsWith(value, "/")) value += "/";
      location.root = value;
    } else if (token == "index") {
      location._index = split(value, " ");
    } else if (token == "autoindex") {
      location._autoindex = (value == "on");
    } else if (token == "cgi") {
      std::vector<std::string> cgi = split(value, " ");
      if (cgi.size() != 2)
        throwExeption("parseLocation", "Expected 2 arguments for cgi");
      location.cgi[cgi[0]] = cgi[1];
    } else {
      throwExeption("parseLocation", "Unknown token '" + token + "'");
    }
    context = trim(context.erase(0, 1));
  }
  return location;
}

bool Config::isContextBlock(const std::string &context) {
  int contextStart = context.find_first_of("{");
  int directiveEnd = context.find_first_of(";");
  if (contextStart == -1) return false;
  if (directiveEnd == -1) return true;
  return contextStart < directiveEnd;
}

int Config::findContextEnd(const std::string &context) {
  int depth = 0;
  int i = -1;
  while (i < (int)context.length()) {
    i = context.find_first_of("{}", i + 1);
    if (i == -1)
      throwExeption("findContextEnd", "Expected token '{' or '}' not found");
    if (context[i] == '{')
      depth++;
    else if (context[i] == '}')
      depth--;
    if (depth == 0) return i + 1;
  }
  throwExeption("findContextEnd", "No context end found");
  return -1;
}

std::string Config::trimContext(const std::string &context) {
  int start = findToken(context, "{");
  int end = context.find_last_of("}");
  if (end == -1) throwExeption("trimContext", "Expected token '}' not found");
  return trim(context.substr(start + 1, end - start - 1));
}

int Config::findToken(const std::string &data, std::string token) {
  int i = data.find(token);
  if (i == -1)
    throwExeption("find_token", "Expected token '" + token + "' not found");
  return i;
}

void Config::throwExeption(std::string func, std::string msg) {
  throw std::runtime_error("Config: " + func + ": " + msg);
}