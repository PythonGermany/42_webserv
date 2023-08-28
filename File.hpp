#ifndef FILE_HPP
#define FILE_HPP

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <stdexcept>
#include <string>

// Class to check file properties and read/write files
class File {
 private:
  std::string _path;

 public:
  File();
  File(std::string path);  // TODO: Add copy constructor
  ~File();

  // Getters
  std::string path();

  // Setters
  void setPath(std::string path);

  bool exists();
  bool file();
  bool dir();
  bool readable();
  long int size();

  // Reads the file
  // @return The file contents
  // @exception std::runtime_error Thrown if the file could not be read
  std::string Read();

  // Writes data to the file
  // @param data The data to write
  // @exception std::runtime_error Thrown if the file could not be written
  void Write(std::string data);

  void append(std::string data); // TODO: implement append

 private:
  // Throws an exeption using the format "File: [func]: [msg]"
  // @param func The function that threw the exeption
  // @param msg The message of the exeption
  // @exception std::runtime_error Thrown with the formatted message
  void throwException(std::string func, std::string msg);
};

#endif