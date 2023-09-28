#ifndef FILE_HPP
#define FILE_HPP

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <ctime>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.hpp"

// Class to check file properties and read/write files
class File {
 private:
  std::string _path;

 public:
  File();
  File(std::string path);
  File(const File &rhs);
  File &operator=(const File &rhs);
  ~File();

  // Getters
  std::string getPath() const;

  std::string getDir() const;
  std::string getName() const;
  std::string getExtension() const;

  // Setters
  void setPath(std::string path);

  // Lists the files in the directory
  // @exception std::runtime_error if the directory cannot be opened
  static std::vector<std::string> list(std::string path);

  // Checks if the file exists
  // @exception No custom exceptions
  bool exists() const;
  // Checks if the file is a regular file
  // @exception No custom exceptions
  bool file() const;
  // Checks if the file is a symbolic link
  // @exception No custom exceptions
  bool isSymLink() const;
  // Checks if the file is a directory
  // @exception No custom exceptions
  bool dir() const;
  // Checks if the file is readable
  // @exception No custom exceptions
  bool readable() const;
  // Checks if the file is writable
  // @exception No custom exceptions
  bool writable() const;
  // Returns the size of the file
  // @exception No custom exceptions
  size_t size() const;
  // Returns the last time the file was modified
  // @exception No custom exceptions
  time_t getLastModified() const;
  // Returns the last time the file was modified
  // @exception No custom exceptions
  std::string lastModified(std::string format) const;

  int resolveSymlink();

  int createDirPath();
};

#endif