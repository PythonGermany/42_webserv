#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <set>
#include <string>
#include <vector>

#include "File.hpp"
#include "Server.hpp"
#include "utils.hpp"

// Parses and validates the config file into a vector of servers
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

  // Getters
  std::string getFile();

  // Setters
  void setFile(std::string path);

  std::vector<Server> parseConfig();

 private:
  Context parseContext(std::string data, std::string name, std::string parent);
  int findContextEnd(const std::string &context);
  int findToken(const std::string &data, std::string token);
  void throwExeption(std::string func, std::string msg);
};

#endif