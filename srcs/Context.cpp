#include "Context.hpp"

Context::Context() {}

Context::Context(std::string name, std::string parent) {
  _name = name;
  _parent = parent;
}

Context::Context(const Context &rhs) { *this = rhs; }

Context &Context::operator=(const Context &rhs) {
  if (this == &rhs) return *this;
  _name = rhs._name;
  _parent = rhs._parent;
  _tokenOccurences = rhs._tokenOccurences;
  _directives = rhs._directives;
  _contexts = rhs._contexts;
  return *this;
}

Context::~Context() {}

std::string Context::getName() { return _name; }

std::string Context::getParent() { return _parent; }

void Context::addDirective(std::string directive,
                           std::vector<std::string> values) {
  if (validArguments(directive, values) == false)
    throwExeption("addDirective",
                  "Invalid directive values'" + directive + "'");
  for (std::vector<std::string>::iterator it = values.begin();
       it != values.end(); it++)
    _directives[directive].push_back(*it);
  addTokenOccurence(directive);
}

void Context::addContext(Context context) {
  _contexts[context.getName()].push_back(context);
  addTokenOccurence(context.getName());
}

size_t Context::getTokenOccurence(std::string token) {
  if (_tokenOccurences.find(token) == _tokenOccurences.end()) return 0;
  return _tokenOccurences[token];
}

std::vector<std::string> Context::getDirective(std::string directive) {
  if (_directives.find(directive) == _directives.end())
    return std::vector<std::string>();
  return _directives[directive];
}

std::vector<Context> Context::getContext(std::string context) {
  if (_contexts.find(context) == _contexts.end()) return std::vector<Context>();
  return _contexts[context];
}

bool Context::isContext(std::string token) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == token && tokens[i].isContext) return true;
  return false;
}

bool Context::isDirective(std::string token) { return !isContext(token); }

bool Context::isValid() {
  writeToLog("Validating '" + _name + "' context" +
                 (_parent == "_" ? "" : " of parent '" + _parent + "'"),
             DEBUG);
  for (std::map<std::string, size_t>::iterator it = _tokenOccurences.begin();
       it != _tokenOccurences.end(); it++) {
    for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++) {
      if (tokens[i].name == it->first) {
        writeToLog("Validating token '" + it->first + "'", DEBUG);
        if (it->second < tokens[i].minOccurence ||
            it->second > tokens[i].maxOccurence) {
          writeToErrorLog("Context '" + _name + "' has " +
                          toString(it->second) + " occurences of token '" +
                          it->first + "' but should have between " +
                          toString(tokens[i].minOccurence) + " and " +
                          toString(tokens[i].maxOccurence));
          return false;
        }
      }
    }
  }
  for (std::map<std::string, std::vector<Context> >::iterator it =
           _contexts.begin();
       it != _contexts.end(); it++) {
    for (std::vector<Context>::iterator it2 = it->second.begin();
         it2 != it->second.end(); it2++) {
      if (it2->isValid() == false) {
        writeToErrorLog("Context '" + it2->getName() + "' is invalid");
        return false;
      }
    }
  }
  writeToLog("Context '" + _name + "' is valid", DEBUG);
  return true;
}

void Context::print(int indent) {
  std::string spaces = "";
  for (int i = 0; i < indent; i++) spaces += "| ";
  std::cout << spaces << RED << _name << RESET << std::endl;
  spaces += "| ";
  for (std::map<std::string, std::vector<std::string> >::iterator it =
           _directives.begin();
       it != _directives.end(); it++) {
    std::cout << spaces << BLUE << it->first << ": " << GREEN;
    for (std::vector<std::string>::iterator it2 = it->second.begin();
         it2 != it->second.end(); it2++)
      std::cout << "'" << *it2 << "' ";
    std::cout << RESET << std::endl;
  }
  for (std::map<std::string, std::vector<Context> >::iterator it =
           _contexts.begin();
       it != _contexts.end(); it++) {
    for (std::vector<Context>::iterator it2 = it->second.begin();
         it2 != it->second.end(); it2++) {
      it2->print(indent + 1);
    }
  }
}

void Context::addTokenOccurence(std::string token) {
  if (_tokenOccurences.find(token) == _tokenOccurences.end())
    _tokenOccurences[token] = 1;
  else
    _tokenOccurences[token]++;
}

bool Context::validArguments(std::string token, std::vector<std::string> args) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++) {
    if (tokens[i].name == token) {
      if (tokens[i].minArgs > args.size() || tokens[i].maxArgs < args.size())
        return false;
      return true;
    }
  }
  return false;
}

void Context::throwExeption(std::string func, std::string msg) {
  throw std::runtime_error("Context: " + func + ": " + msg);
}