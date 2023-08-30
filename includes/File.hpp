#ifndef FILE_HPP
#define FILE_HPP

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>

// Class to check file properties and read/write files
class File {
 private:
  static int _filesOpen;
  int _fd;
  std::string _path;

 public:
  File();
  File(std::string path);
  File(const File &rhs);
  File &operator=(const File &rhs);
  ~File();

  static int getFilesOpen();

  // Getters
  std::string getPath();
  std::string getExtension();

  // Setters
  void setPath(std::string path);

  bool exists();
  bool file();
  bool dir();
  bool readable();
  bool writable();
  long int size();

  bool isOpen();

  void create();
  void open(int flags = O_RDONLY, mode_t mode = 0644);
  void close();

  std::string read();
  void write(std::string data);

 private:
  void throwException(std::string func, std::string msg);
};

#endif