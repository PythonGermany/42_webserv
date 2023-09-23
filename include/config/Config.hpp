#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <set>
#include <string>
#include <vector>

#include "Context.hpp"
#include "File.hpp"
#include "Log.hpp"
#include "utils.hpp"
#include "webserv.hpp"

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
  // Sets the config file
  // @exception std::runtime_error If the file is not found or not readable or
  // if there is an error while reading the file
  void setFile(std::string path);

  // Getters
  std::string getConfig() const;

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
  void processInclude(Context &context, std::string path);

  // Checks if the token is a valid context in the given context
  // @param context The context to check in
  // @param token The token to check
  // @return True if the token is valid in the context, false otherwise
  // @exception No custom exceptions
  bool isValidContext(Context &context, std::string token) const;

  // Checks if the token is a valid directive in the given context
  // @param context The context to check in
  // @param token The token to check
  // @return True if the token is valid in the context, false otherwise
  // @exception No custom exceptions
  bool isValidDirective(Context &context, std::string token) const;

  // Checks if the token is valid to add to the context
  // @param context The context to check in
  // @param token The token to check
  // @return An empty string if the token is valid, an error message otherwise
  // @exception No custom exceptions
  std::string validToAdd(Context &context, std::string token);

  // Checks if the arguments are valid for the given token
  // @param context The context to check in
  // @param token The token to check
  // @param args The arguments to check
  // @return An empty string if the arguments are valid, an error message
  // otherwise
  // @exception No custom exceptions
  std::string validArguments(Context &context, std::string token,
                             std::vector<std::string> args);

  // Validates the context
  // @param context The context to validate
  // @param recursive If true, the function will validate all child contexts
  // @return An empty string if the context is valid, an error message otherwise
  // @exception No custom exceptions
  std::string validate(Context &context, bool recursive);

  // Returns the number of lines until the given position
  // @param data The string to search in
  // @param pos The position to search until
  // @return The number of lines until the given position
  // @exception No custom exceptions
  int linesUntilPos(const std::string &data, size_t pos) const;

  // Returns the position of the end of the context
  // @param context The context to search in
  // @return The position of the end of the context
  // @exception No custom exceptions
  size_t findContextEnd(const std::string &context) const;

 private:
  // Checks if error is not empty and throws an exeption with the given message
  // and line
  void checkError(size_t line, std::string error);

  // Throws an exeption with the given message and line
  // @param line The line in the config file where the error occured
  // @param msg The message to display
  // @return std::runtime_error
  void throwExeption(size_t line, std::string msg);
};

#endif