#include "Context.hpp"

Context::Context() {}

Context::Context(std::string name, Context *parent) {
  _name = name;
  _parent = parent;
}

Context::Context(const Context &rhs) { *this = rhs; }

Context &Context::operator=(const Context &rhs) {
  if (this == &rhs) return *this;
  _name = rhs._name;
  _parent = rhs._parent;
  _args = rhs._args;
  _tokenOccurences = rhs._tokenOccurences;
  _directives = rhs._directives;
  _contexts = rhs._contexts;

  // Update parent pointers of child contexts
  for (std::map<std::string, std::vector<Context> >::iterator it =
           _contexts.begin();
       it != _contexts.end(); it++)
    for (size_t i = 0; i < it->second.size(); i++)
      it->second[i].setParent(this);
  return *this;
}

Context::~Context() {}

std::string Context::getName() { return _name; }

std::string Context::getParent() {
  return _parent == NULL ? "" : _parent->getName();
}

std::vector<std::string> &Context::getArgs() { return _args; }

size_t Context::getTokenOccurence(std::string token) {
  if (_tokenOccurences.find(token) == _tokenOccurences.end()) return 0;
  return _tokenOccurences[token];
}

std::map<std::string, size_t> Context::getTokenOccurences() {
  return _tokenOccurences;
}

std::vector<std::vector<std::string> > &Context::getDirective(std::string token,
                                                              bool searchTree) {
  if (exists(token, false)) return _directives[token];
  if (searchTree && _parent != NULL)
    return _parent->getDirective(token, searchTree);
  return _directives[token];
}

std::vector<Context> &Context::getContext(std::string token, bool searchTree) {
  if (exists(token, false)) return _contexts[token];
  if (searchTree && _parent != NULL)
    return _parent->getContext(token, searchTree);
  return _contexts[token];
}

std::map<std::string, std::vector<Context> > &Context::getContexts() {
  return _contexts;
}

void Context::setParent(Context *parent) { _parent = parent; }

void Context::setArgs(std::vector<std::string> args) { _args = args; }

void Context::setTokenOccurences(
    std::map<std::string, size_t> tokenOccurences) {
  _tokenOccurences = tokenOccurences;
}

void Context::addDirective(std::string token, std::vector<std::string> values) {
  _directives[token].push_back(values);
  addTokenOccurence(token);
}

std::string Context::addContext(Context &context) {
  _contexts[context.getName()].push_back(context);
  addTokenOccurence(context.getName());
  return "";
}

void Context::removeDirective(std::string token) {
  _directives.erase(token);
  _tokenOccurences.erase(token);
}

void Context::removeContext(std::string token) {
  _contexts.erase(token);
  _tokenOccurences.erase(token);
}

size_t Context::argCount() const { return _args.size(); }

bool Context::exists(std::string token, bool searchTree) const {
  if (_tokenOccurences.find(token) != _tokenOccurences.end()) return true;
  if (searchTree && _parent != NULL) return _parent->exists(token, searchTree);
  return false;
}

void Context::print(int indent) const {
  std::string spaces = GRAY;
  for (int i = 0; i < indent; i++) spaces += "| ";
  std::cout << spaces << RED << _name;
  spaces += "| ";
  spaces += RESET;

  // Print arguments
  if (_args.size() > 0) {
    std::cout << " " << GREEN;
    for (size_t i = 0; i < _args.size(); i++) std::cout << _args[i] << " ";
  }
  std::cout << RESET << std::endl;

  // Print directives
  for (std::map<std::string,
                std::vector<std::vector<std::string> > >::const_iterator it =
           _directives.begin();
       it != _directives.end(); it++) {
    for (size_t i = 0; i < it->second.size(); i++) {
      std::cout << spaces << BLUE << it->first << " " << GREEN;
      for (size_t j = 0; j < it->second[i].size(); j++)
        std::cout << it->second[i][j] << " ";
      std::cout << RESET << std::endl;
    }
  }

  // Recursively print contexts
  for (std::map<std::string, std::vector<Context> >::const_iterator it =
           _contexts.begin();
       it != _contexts.end(); it++) {
    for (size_t i = 0; i < it->second.size(); i++)
      it->second[i].print(indent + 1);
  }
}

void Context::addTokenOccurence(std::string token) {
  if (_tokenOccurences.find(token) == _tokenOccurences.end())
    _tokenOccurences[token] = 1;
  else
    _tokenOccurences[token]++;
}