#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Log.hpp"
#include "colors.hpp"
#include "utils.hpp"
#include "../webserv.hpp"

class Context {
 private:
  std::string _name;
  std::string _parent;
  std::map<std::string, size_t> _tokenOccurences;
  std::map<std::string, std::vector<std::string> > _directives;
  std::map<std::string, std::vector<Context> > _contexts;

 public:
  Context();
  Context(std::string name, std::string parent);
  Context(const Context &rhs);
  Context &operator=(const Context &rhs);
  ~Context();

  // Getters
  std::string getName();
  std::string getParent();
  size_t getTokenOccurence(std::string token);
  std::map<std::string, size_t> getTokenOccurences();
  std::vector<std::string> &getDirective(std::string token);
  std::vector<Context> &getContext(std::string token);

  // Setters/Adders
  void setTokenOccurences(std::map<std::string, size_t> tokenOccurences);
  std::string addDirective(std::string token, std::vector<std::string> values);
  std::string addContext(Context context);

  // Remove
  void removeDirective(std::string token);
  void removeContext(std::string token);

  // Checks if the token exists in the context
  // @exception No custom exceptions
  bool exists(std::string token);

  // Checks if the token is a valid context token
  // @exception No custom exceptions
  bool isValidContext(std::string token);
  // Checks if the token is a valid directive token
  // @exception No custom exceptions
  bool isValidDirective(std::string token);
  // Checks if the token is a valid token and if it can be added to the context
  // @return Empty string if the token is valid, an error message otherwise
  // @exception No custom exceptions
  std::string validToAdd(std::string token);
  // Checks if the arguments are valid for the token
  // @return Empty string if the arguments are valid, an error message otherwise
  // @exception No custom exceptions
  std::string validArguments(std::string token, std::vector<std::string> args);

  // Checks if the context is valid
  // @return Empty string if the context is valid, an error message otherwise
  // @exception No custom exceptions
  std::string validate(bool recursive = true);

  // Prints the context to the standard output
  // @exception No custom exceptions
  void print(int indent = 0);

 private:
  // Adds a token occurence to the member _tokenOccurences
  // @exception No custom exceptions
  void addTokenOccurence(std::string token);
};

#endif