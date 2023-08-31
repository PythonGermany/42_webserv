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

  // Setters
  void setFile(std::string path);

  // Getters
  std::string getConfig();

  void removeComments();
  Context parseContext(std::string data, std::string name, std::string parent,
                       size_t line = 1);

 private:
  int linesUntilPos(const std::string &data, size_t pos);
  size_t findContextEnd(const std::string &context);
  void throwExeption(size_t line, std::string msg);
};

#endif