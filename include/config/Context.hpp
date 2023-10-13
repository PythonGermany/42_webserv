#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "colors.hpp"
#include "utils.hpp"

class Context {
 private:
  std::string _name;
  Context *_parent;
  std::vector<std::string> _args;
  std::map<std::string, size_t> _tokenOccurences;
  std::map<std::string, std::vector<std::vector<std::string> > > _directives;
  std::map<std::string, std::vector<Context> > _contexts;

 public:
  Context();
  Context(std::string name, Context *parent = NULL);
  Context(const Context &rhs);
  Context &operator=(const Context &rhs);
  ~Context();

  // Getters
  std::string getName();
  std::string getParent();
  std::vector<std::string> &getArgs();
  size_t getTokenOccurence(std::string token);
  std::map<std::string, size_t> getTokenOccurences();
  std::vector<std::vector<std::string> > &getDirective(std::string token,
                                                       bool searchTree = false);
  std::vector<Context> &getContext(std::string token, bool searchTree = false);
  std::map<std::string, std::vector<Context> > &getContexts();

  void setParent(Context *parent);
  void setArgs(std::vector<std::string> args);
  void setTokenOccurences(std::map<std::string, size_t> tokenOccurences);
  void addDirective(std::string token, std::vector<std::string> values);
  std::string addContext(Context &context);

  // Removes all directives with the given token
  // @exception No custom exceptions
  void removeDirective(std::string token);
  // Removes all contexts with the given token
  // @exception No custom exceptions
  void removeContext(std::string token);

  // Returns the number of context arguments
  size_t argCount() const;

  // Checks if the token exists in the context
  // @param searchTree If true, the function will search its parent tree
  // @exception No custom exceptions
  bool exists(std::string token, bool searchTree = false) const;

  // Returns the structure of the parsed config as a string
  // @exception No custom exceptions
  std::string getStructure(int indent = 0) const;

 private:
  // Adds a token occurence to the member _tokenOccurences
  // @exception No custom exceptions
  void addTokenOccurence(std::string token);
};

#endif