#include "Config.hpp"

Config::Config() {}

Config::Config(std::string path) {
  _file = File(path);
  if (!_file.exists()) throwExeption("Config", "File does not exist");
  if (!_file.file()) throwExeption("Config", "Path is not a file");
  if (!_file.readable()) throwExeption("Config", "File is not readable");
  _config = _file.Read();
}

Config::Config(const Config &rhs) { *this = rhs; }

Config &Config::operator=(const Config &rhs) {
  if (this == &rhs) return *this;
  _file = rhs._file;
  return *this;
}

Config::~Config() {}

#include <iostream>  // TODO: remove

std::vector<Server> Config::parse() {
  std::vector<Server> servers;
  std::string data = _config;

  while (data.length() > 0) {
    int end = findContextEnd(data);
    if (end == -1) throwExeption("parse", "Expected token '}' not found");
    std::string context = data.substr(0, end + 1);
    servers.push_back(parseServer(context));
    data = data.substr(end + 1);
  }
  return servers;
}

Server Config::parseServer(std::string context) {
  Server server;
  if (findToken(context, "server") != 0)
    throwExeption("parseServer", "Expected token 'server' not found");
  context = context.substr(6, context.length());
  context = trim(context);
  context = trim(context.substr(1, context.length() - 2));
  while (context.length() > 0) {
    if (isContext(context))
      parseContext(cut(context, 0, findContextEnd(context) + 1), server);
    else
      parseDirective(cut(context, 0, findToken(context, ";") + 1), server);
  }
  return server;
}

void Config::parseContext(std::string context, Server &server) {
  context = trim(context.substr(0, findToken(context, "{")));
  if (context == "server") {
    throwExeption("parseContext", "Nested server context not allowed");
  } else if (context == "location") {
    location location = parseLocation(context);
    server.addLocation(location);
  } else {
    throwExeption("parseContext", "Unknown context '" + context + "'");
  }
}

void Config::parseDirective(std::string directive, Server &server) {
  // TODO: parse directive
}

location Config::parseLocation(std::string context) {
  location location;
  // TODO: parse location context
  return location;
}

bool Config::isContext(const std::string &context) {
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
    if (i == -1) return -1;
    if (context[i] == '{')
      depth++;
    else if (context[i] == '}')
      depth--;
    if (depth == 0) return i;
  }
  return -1;
}

int Config::findToken(const std::string &data, std::string token) {
  int i = data.find(token);
  if (i == -1)
    throwExeption("find_token", "Expected token '" + token + "' not found");
  return i;
}

void Config::throwExeption(std::string func, std::string msg) {
  throw std::runtime_error("Config::" + func + ": " + msg);
}