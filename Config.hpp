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

 private:
  Server parseServer(std::string block);
  location parseLocation(std::string block);
  int findBlockEnd(std::string block);
  int findToken(std::string data, std::string token);

  void throwExeption(std::string func, std::string msg);
};

#endif