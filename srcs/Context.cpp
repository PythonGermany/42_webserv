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

size_t Context::getTokenOccurence(std::string token) {
  if (_tokenOccurences.find(token) == _tokenOccurences.end()) return 0;
  return _tokenOccurences[token];
}

std::string Context::addDirective(std::string token,
                                  std::vector<std::string> values) {
  std::string error = validToAdd(token);
  if (error.size() > 0) return error;
  error = validArguments(token, values);
  if (error.size() > 0) return error;
  for (std::vector<std::string>::iterator it = values.begin();
       it != values.end(); it++)
    _directives[token].push_back(*it);
  addTokenOccurence(token);
  return "";
}

std::string Context::addContext(Context context) {
  std::string error = validToAdd(context.getName());
  if (error.size() > 0) return error;
  _contexts[context.getName()].push_back(context);
  addTokenOccurence(context.getName());
  return "";
}

bool Context::exists(std::string token) {
  return _tokenOccurences.find(token) != _tokenOccurences.end();
}

std::vector<std::string> Context::getDirective(std::string token) {
  if (_directives.find(token) == _directives.end())
    return std::vector<std::string>();
  return _directives[token];
}

std::vector<Context> Context::getContext(std::string token) {
  if (_contexts.find(token) == _contexts.end()) return std::vector<Context>();
  return _contexts[token];
}

bool Context::isValidContext(std::string token) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == token && tokens[i].parent == _name &&
        tokens[i].isContext)
      return true;
  return false;
}

bool Context::isValidDirective(std::string token) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == token && tokens[i].parent == _name &&
        !tokens[i].isContext)
      return true;
  return false;
}

std::string Context::validToAdd(std::string token) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == token && tokens[i].parent == _name)
      return getTokenOccurence(token) < tokens[i].maxOccurence
                 ? ""
                 : "Token '" + token + "'has too many occurences";
  return "Token '" + token + "' not found";
}

std::string Context::validArguments(std::string token,
                                    std::vector<std::string> args) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++) {
    if (tokens[i].name == token && tokens[i].parent == _name) {
      if (tokens[i].func != NULL) {
        for (std::vector<std::string>::iterator it = args.begin();
             it != args.end(); it++) {
          std::string error = tokens[i].func(*it);
          if (error != "") return "Argument '" + *it + "': " + error;
        }
      }
      if (tokens[i].minArgs <= args.size() && tokens[i].maxArgs >= args.size())
        return "";
      return "'" + token + "' requires between " + toString(tokens[i].minArgs) +
             " and " + toString(tokens[i].maxArgs) + " arguments";
    }
  }
  return "Token '" + token + "' not found";
}

std::string Context::validate(bool recursive) {
  Log::write("Context: '" + _name + "' -> Validating", DEBUG);
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].parent == _name &&
        getTokenOccurence(tokens[i].name) < tokens[i].minOccurence)
      return "Missing required directive '" + tokens[i].name + "'";
  if (recursive) {
    for (std::map<std::string, std::vector<Context> >::iterator it =
             _contexts.begin();
         it != _contexts.end(); it++) {
      for (std::vector<Context>::iterator it2 = it->second.begin();
           it2 != it->second.end(); it2++) {
        it2->validate();
      }
    }
  }
  Log::write("Context: '" + _name + "' -> Sucessfully validated", DEBUG);
  return "";
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

void Context::throwExeption(std::string func, std::string msg) {
  throw std::runtime_error("Context: " + func + ": " + msg);
}