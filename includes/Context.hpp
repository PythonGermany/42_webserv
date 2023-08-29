#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>

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
  std::vector<std::string> getDirective(std::string directive);
  std::vector<Context> getContext(std::string context);

  // Setters/Adders
  void addDirective(std::string directive, std::vector<std::string> values);
  void addContext(Context context);

  bool isValidToken(std::string token);
  bool isContext(std::string token);
  bool isDirective(std::string token);
  bool isValid();

  void print(int indent = 0);

 private:
  void addTokenOccurence(std::string token);
  bool validArguments(std::string token, std::vector<std::string> args);
  void throwExeption(std::string func, std::string msg);
};

#endif