#ifndef FILE_HPP
#define FILE_HPP

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <stdexcept>
#include <string>

class File {
 private:
  std::string _path;

 public:
  File();
  File(std::string path);
  ~File();

  bool exists();
  bool file();
  bool dir();
  bool readable();
  long int size();

  std::string Read();
  void Write(std::string data);

 private:
  void throwException(std::string func, std::string msg);
};

#endif