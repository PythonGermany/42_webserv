#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>

#include "File.hpp"
#include "Server.hpp"
#include "utils.hpp"

class Config {
 private:
  File _file;
  std::string _config;

 public:
  Config();
  Config(std::string path);
  Config(const Config &rhs);
  Config &operator=(const Config &rhs);
  ~Config();

  std::vector<Server> parse();
  void validate(std::vector<Server> &servers);

 private:
  Server parseServer(std::string context);
  void parseContext(std::string context, Server &server);
  location parseLocation(std::string context);
  bool isContextBlock(const std::string &context);
  int findContextEnd(const std::string &context);
  std::string trimContext(const std::string &context);
  int findToken(const std::string &data, std::string token);

  void throwExeption(std::string func, std::string msg);
};

#endif