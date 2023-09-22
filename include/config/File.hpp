#ifndef FILE_HPP
#define FILE_HPP

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

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

  // Getters
  static int getFilesOpen();
  std::string getPath();
  std::string getExtension();

  std::string getDir();

  // Setters
  void setPath(std::string path);

  // Lists the files in the directory
  // @exception std::runtime_error if the directory cannot be opened
  static std::vector<std::string> list(std::string path);

  // Checks if the file exists
  // @exception No custom exceptions
  bool exists();
  // Checks if the file is a regular file
  // @exception No custom exceptions
  bool file();
  // Checks if the file is a directory
  // @exception No custom exceptions
  bool dir();
  // Checks if the file is readable
  // @exception No custom exceptions
  bool readable();
  // Checks if the file is writable
  // @exception No custom exceptions
  bool writable();
  // Returns the size of the file
  // @exception No custom exceptions
  size_t size();
  // Returns the last time the file was modified
  // @exception No custom exceptions
  std::string lastModified(std::string format = "%d-%m-%Y %H:%M");

  // Checks if the file has been opened through member function open()
  // @exception No custom exceptions
  bool isOpen();

  // Creates the file if it doesn't exist and the directories leading to it if
  // they don't exist
  // @exception std::runtime_error if the creation of the file or the
  // directories fails
  void create();
  // Deletes the file
  // @exception std::runtime_error if the deletion of the file fails
  void remove();
  // Opens the file
  // @exception std::runtime_error if opening the file fails
  void open(int flags, mode_t mode = 0644);
  // Closes the file
  // @exception std::runtime_error if closing the file fails
  void close();

  // Reads the file
  // @exception std::runtime_error if reading the file fails
  std::string read();
  // Writes to the file
  // @exception std::runtime_error if writing to the file fails
  void write(std::string data);

 private:
  // Throws an exception indicating the function and message
  // @exception std::runtime_error
  static void throwException(std::string func, std::string msg);
};

#endif