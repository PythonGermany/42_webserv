#ifndef CGI_HPP
#define CGI_HPP

#include <string>

class Cgi {
 private:
  std::string _path;

 public:
  Cgi();
  Cgi(std::string path);  // TODO: Add copy constructor
  ~Cgi();

  std::string getPath();

  std::string executeFile(std::string path);
};

#endif