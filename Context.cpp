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
  _directives = rhs._directives;
  _contexts = rhs._contexts;
  return *this;
}

Context::~Context() {}

std::string Context::getName() { return _name; }

std::string Context::getParent() { return _parent; }

void Context::addDirective(std::string directive, std::string value) {
  _directives[directive].push_back(value);
}

void Context::addDirectives(std::string directive,
                            std::vector<std::string> values) {
  for (std::vector<std::string>::iterator it = values.begin();
       it != values.end(); it++) {
    _directives[directive].push_back(*it);
  }
}

void Context::addContext(Context context) {
  _contexts[context.getName()].push_back(context);
}

void Context::addContexts(std::vector<Context> contexts) {
  for (std::vector<Context>::iterator it = contexts.begin();
       it != contexts.end(); it++) {
    _contexts[it->getName()].push_back(*it);
  }
}

bool Context::validTokenName(std::string name) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == name) return true;
  return false;
}

bool Context::isContext(std::string name) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == name && tokens[i].isContext) return true;
  return false;
}

bool Context::isDirective(std::string name) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == name && !tokens[i].isContext) return true;
  return false;
}

bool Context::validDirective(std::string name,
                             std::vector<std::string> values) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++) {
    if (tokens[i].name == name) {
      if (tokens[i].minArgs > values.size() ||
          tokens[i].maxArgs < values.size())
        return false;
      return true;
    }
  }
  return false;
}

void Context::print(int indent) {
  std::string spaces = "";
  for (int i = 0; i < indent; i++) spaces += "| ";
  std::cout << spaces << _name << std::endl;
  spaces += "| ";
  for (std::map<std::string, std::vector<std::string> >::iterator it =
           _directives.begin();
       it != _directives.end(); it++) {
    std::cout << spaces << it->first << ": ";
    for (std::vector<std::string>::iterator it2 = it->second.begin();
         it2 != it->second.end(); it2++)
      std::cout << "'" << *it2 << "' ";
    std::cout << std::endl;
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