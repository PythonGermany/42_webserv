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
    int end = findBlockEnd(data);
    if (end == -1) throwExeption("parse", "Expected token '}' not found");
    std::string block = data.substr(0, end + 1);
    servers.push_back(parseServer(block));
    data = data.substr(end + 1);
  }
  return servers;
}

Server Config::parseServer(std::string block) {
  Server server;
  if (findToken(block, "server") != 0)
    throwExeption("parseServer", "Expected token 'server' not found");
  block = block.substr(6, block.length());
  block = trim(block);
  block = trim(block.substr(1, block.length() - 2));
  while (block.length() > 0) {
    // TODO: parse server block
  }
  return server;
}

location Config::parseLocation(std::string block) {
  location location;
  // TODO: parse location block
  return location;
}

int Config::findBlockEnd(std::string block) {
  int depth = 0;
  int i = -1;
  while (i < (int)block.length()) {
    i = block.find_first_of("{}", i + 1);
    if (i == -1) return -1;
    if (block[i] == '{')
      depth++;
    else if (block[i] == '}')
      depth--;
    if (depth == 0) return i;
  }
  return -1;
}

int Config::findToken(std::string data, std::string token) {
  int i = data.find(token);
  if (i == -1)
    throwExeption("find_token", "Expected token '" + token + "' not found");
  return i;
}

void Config::throwExeption(std::string func, std::string msg) {
  throw std::runtime_error("Config::" + func + ": " + msg);
}