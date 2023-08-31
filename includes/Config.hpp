#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <set>
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

  // Setters
  void setFile(std::string path);

  // Getters
  std::string getConfig();

  // Removes all comments from the config file
  // @exception No custom exceptions
  void removeComments();
  // Recursively parses a context
  // @param Data the data to parse
  // @param Name the name of the root context
  // @param The name of the parent context
  // @return The parsed context object
  // @exception std::runtime_error if the context is invalid
  Context parseContext(std::string data, std::string name,
                       std::string parent = "", size_t line = 1);

 private:
  // Returns the number of lines until the given position
  // @param The string to search in
  // @param The position to search until
  // @return The number of lines until the given position
  // @exception No custom exceptions
  int linesUntilPos(const std::string &data, size_t pos);
  // Returns the position of the end of the context
  // @param The context to search in
  // @return The position of the end of the context
  // @exception No custom exceptions
  size_t findContextEnd(const std::string &context);
  // Throws an exeption with the given message and line
  // @param The line in the config file where the error occured
  // @param The message to display
  // @return std::runtime_error
  void throwExeption(size_t line, std::string msg);
};

#endif