#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Log.hpp"
#include "colors.hpp"
#include "utils.hpp"
#include "webserv.hpp"

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
  std::vector<std::string> getDirective(std::string token);
  std::vector<Context> getContext(std::string token);

  // Setters/Adders
  std::string addDirective(std::string token, std::vector<std::string> values);
  std::string addContext(Context context);

  bool exists(std::string token);

  bool isValidContext(std::string token);
  bool isValidDirective(std::string token);
  std::string validToAdd(std::string token);
  std::string validArguments(std::string token, std::vector<std::string> args);

  std::string validate(bool recursive = true);

  void print(int indent = 0);

 private:
  void addTokenOccurence(std::string token);
  void throwExeption(std::string func, std::string msg);
};

#endif