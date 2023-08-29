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

  // Parses the config file
  // @return The parsed servers
  // @exception std::runtime_error Thrown if the config is invalid
  std::vector<Server> parseConfig();

  // Validates the configuration of the parsed servers
  // @param servers The servers to validate
  // @exception std::runtime_error Thrown if the config is invalid
  void validateConfig(std::vector<Server> &servers);

 private:
  Context parseContext(std::string data, std::string name, std::string parent);

  // Finds the end of the context block
  // @param context The context to find the end of
  // @return The index of the end of the context block
  // @exception std::runtime_error Thrown if the context is not a block or the
  // end could not be found
  int findContextEnd(const std::string &context);

  // Finds the token
  // @param data The data to find the token in
  // @param token The token to find
  // @return The index of the fist occurence of the token
  // @exception std::runtime_error Thrown if the token could not be found
  int findToken(const std::string &data, std::string token);

  // Throws an exeption using the format "Config: [func]: [msg]"
  // @param func The function that threw the exeption
  // @param msg The message of the exeption
  // @exception std::runtime_error Thrown with the formatted message
  void throwExeption(std::string func, std::string msg);
};

#endif