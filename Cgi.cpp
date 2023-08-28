#include "Cgi.hpp"

Cgi::Cgi() {}

Cgi::Cgi(std::string path) { _path = path; }

Cgi::~Cgi() {}

std::string Cgi::getPath() { return _path; }

std::string Cgi::executeFile(std::string path) {
  (void)path;   // TODO: implement executing cgi
  return path;  // TODO: Replace with output
}