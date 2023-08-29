#ifndef CGI_HPP
#define CGI_HPP

#include <unistd.h>

#include <string>
#include <map>
#include <stdexcept>

#include "File.hpp"

class Cgi {
 private:
  std::map<std::string, std::string> _entries;

 public:
  Cgi();
  ~Cgi();

  void addEntry(std::string key, std::string value);

  std::string getEntry(std::string key);
  std::map<std::string, std::string> getEntries();

  bool isCgiFile(File &file);
  std::string executeFile(File &file, std::string pwd);

private:
  void throwException(std::string func, std::string msg);
};

#endif