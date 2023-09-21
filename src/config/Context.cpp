#include "Context.hpp"

Context::Context() {}

Context::Context(std::string name, Context *parent) {
  _name = name;
  _parentContext = parent;
  if (_parentContext == NULL)
    _parent = "";
  else
    _parent = _parentContext->getName();
}

Context::Context(const Context &rhs) { *this = rhs; }

Context &Context::operator=(const Context &rhs) {
  if (this == &rhs) return *this;
  _name = rhs._name;
  _parent = rhs._parent;
  _parentContext = rhs._parentContext;
  _args = rhs._args;
  _tokenOccurences = rhs._tokenOccurences;
  _directives = rhs._directives;
  _contexts = rhs._contexts;
  return *this;
}

Context::~Context() {}

std::string Context::getName() { return _name; }

std::string Context::getParent() { return _parent; }

std::vector<std::string> &Context::getArgs() { return _args; }

size_t Context::getTokenOccurence(std::string token) {
  if (_tokenOccurences.find(token) == _tokenOccurences.end()) return 0;
  return _tokenOccurences[token];
}

std::map<std::string, size_t> Context::getTokenOccurences() {
  return _tokenOccurences;
}

std::vector<std::string> &Context::getDirective(std::string token,
                                                bool searchTree) {
  if (exists(token, false)) return _directives[token];
  if (searchTree && _parentContext != NULL)
    return _parentContext->getDirective(token, searchTree);
  return _directives[token];
}

std::vector<Context> &Context::getContext(std::string token) {
  return _contexts[token];
}

void Context::setParent(Context *parent) {
  _parentContext = parent;
  if (_parentContext == NULL)
    _parent = "";
  else
    _parent = _parentContext->getName();
}

void Context::setArgs(std::vector<std::string> args) { _args = args; }

void Context::setTokenOccurences(
    std::map<std::string, size_t> tokenOccurences) {
  _tokenOccurences = tokenOccurences;
}

std::string Context::addDirective(std::string token,
                                  std::vector<std::string> values) {
  std::string error = validToAdd(token);
  if (error.size() > 0) return error;
  error = validArguments(token, values);
  if (error.size() > 0) return error;
  for (size_t i = 0; i < values.size(); i++)
    _directives[token].push_back(values[i]);
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

void Context::removeDirective(std::string token) {
  _directives.erase(token);
  _tokenOccurences.erase(token);
}

void Context::removeContext(std::string token) {
  _contexts.erase(token);
  _tokenOccurences.erase(token);
}

size_t Context::argCount() { return _args.size(); }

bool Context::exists(std::string token, bool searchTree) {
  if (_tokenOccurences.find(token) != _tokenOccurences.end()) return true;
  if (searchTree && _parentContext != NULL)
    return _parentContext->exists(token, searchTree);
  return false;
}

bool Context::isValidContext(std::string token) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == token && tokens[i].parent == _name &&
        tokens[i].isContext)
      return true;
  return false;
}

std::string Context::isValidContextArgs(std::vector<std::string> args) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++) {
    if (tokens[i].name == _name && tokens[i].isContext) {
      if (tokens[i].func != NULL) {
        for (size_t j = 0; j < args.size(); j++) {
          std::string error = tokens[i].func(args[j]);
          if (error != "") return "Argument '" + args[j] + "': " + error;
        }
      }
      if (tokens[i].minArgs <= args.size() && tokens[i].maxArgs >= args.size())
        return "";
      return "'" + _name + "' requires between " + toString(tokens[i].minArgs) +
             " and " + toString(tokens[i].maxArgs) + " arguments";
    }
  }
  return "Context '" + _name + "' not found";
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
        for (size_t j = 0; j < args.size(); j++) {
          std::string error = tokens[i].func(args[j]);
          if (error != "") return "Argument '" + args[j] + "': " + error;
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
      return "Context missing required directive '" + tokens[i].name + "'";
  if (recursive) {
    std::map<std::string, std::vector<Context> >::iterator it;
    for (it = _contexts.begin(); it != _contexts.end(); it++)
      for (size_t i = 0; i < it->second.size(); i++) it->second[i].validate();
  }
  Log::write("Context: '" + _name + "' -> Sucessfully validated", DEBUG);
  return "";
}

void Context::print(int indent) {
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
  for (std::map<std::string, std::vector<std::string> >::iterator it =
           _directives.begin();
       it != _directives.end(); it++) {
    std::cout << spaces << BLUE << it->first << ": " << GREEN;
    for (size_t i = 0; i < it->second.size(); i++)
      std::cout << "'" << it->second[i] << "' ";
    std::cout << RESET << std::endl;
  }

  // Recursively print contexts
  for (std::map<std::string, std::vector<Context> >::iterator it =
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