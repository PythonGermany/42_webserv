#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>

typedef struct s_token {  // TODO: Test if this is a good idea
  std::string name;
  std::string parent;
  bool isContext;
  size_t minOccurence;
  size_t maxOccurence;
  size_t minArgs;
  size_t maxArgs;
} t_token;

const t_token tokens[15] = {
    {"server", "_", true, 1, -1, 0, 0},
    {"host", "server", false, 1, -1, 1, 1},
    {"port", "server", false, 1, -1, 1, -1},
    {"server_name", "server", false, 0, -1, 1, -1},
    {"error_page", "server", false, 0, 0, 2, 2},
    {"client_max_body_size", "server", false, 0, 1, 1, 1},
    {"location", "server", true, 1, -1, 0, 0},
    {"url", "location", false, 1, 0, 1, 1},
    {"method", "location", false, 0, 0, 1, -1},
    {"root", "location", false, 0, 1, 1, 1},
    {"index", "location", false, 0, 0, 1, -1},
    {"autoindex", "location", false, 0, 1, 1, 1},
    {"upload", "location", false, 0, 1, 1, 1},
    {"cgi", "location", false, 0, 1, 2, 2},
    {"redirect", "location", false, 0, 1, 1, 1},
};

class Context {
 private:
  std::string _name;
  std::string _parent;
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

  // Setters/Adders
  void addDirective(std::string directive, std::string value);
  void addDirectives(std::string directive, std::vector<std::string> values);
  void addContext(Context context);
  void addContexts(std::vector<Context> contexts);

  bool validTokenName(std::string name);
  bool isContext(std::string name);
  bool isDirective(std::string name);
  bool validDirective(std::string name, std::vector<std::string> values);

  void print(int indent = 0);
};

#endif