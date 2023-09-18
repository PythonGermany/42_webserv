#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <set>
#include <string>
#include <vector>

#include "Context.hpp"
#include "File.hpp"
#include "utils.hpp"

class Config {
 private:
  File _file;
  std::string _config;
  std::string _error;

 public:
  Config();
  Config(std::string path);
  Config(const Config &rhs);
  Config &operator=(const Config &rhs);
  ~Config();

  // Setters
  // Sets the config file
  // @exception std::runtime_error If the file is not found or not readable or
  // if there is an error while reading the file
  void setFile(std::string path);

  // Getters
  std::string getConfig();

  // Removes all comments from the config file
  // @exception No custom exceptions
  void removeComments();
  // Recursively parses a context
  // @param context The context to add data to
  // @param data The data to parse
  // @param line The line in the config file where the context starts
  // @return The parsed context object
  // @exception std::runtime_error If the context is invalid
  Context &parseContext(Context &context, std::string data, size_t line = 1,
                        bool validate = true);

 private:
  // Processes a context
  // @param context The context to add data to
  // @param data The data to process
  // @param token The token name of the context to process
  // @param line The line in the config file where the context starts
  // @exception std::runtime_error If the context is invalid
  void processContext(Context &context, std::string &data, std::string token,
                      size_t &line);
  // Processes an include directive
  // @param context The context to add the included context to
  // @param path The path to the included config file
  std::string processInclude(Context &context, std::string path);
  // Returns the number of lines until the given position
  // @param data The string to search in
  // @param pos The position to search until
  // @return The number of lines until the given position
  // @exception No custom exceptions
  int linesUntilPos(const std::string &data, size_t pos);
  // Returns the position of the end of the context
  // @param context The context to search in
  // @return The position of the end of the context
  // @exception No custom exceptions
  size_t findContextEnd(const std::string &context);
  // Throws an exeption with the given message and line
  // @param line The line in the config file where the error occured
  // @param msg The message to display
  // @return std::runtime_error
  void throwExeption(size_t line, std::string msg);
};

#endif