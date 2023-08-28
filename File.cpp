#include "File.hpp"

File::File() {}

File::File(std::string path) { _path = path; }

File::~File() {}

std::string File::path() { return _path; }

void File::setPath(std::string path) { _path = path; }

bool File::exists() {
  struct stat buf;
  return stat(_path.c_str(), &buf) == 0;
}

bool File::file() {
  struct stat buf;
  stat(_path.c_str(), &buf);
  return S_ISREG(buf.st_mode);
}

bool File::dir() {
  struct stat buf;
  stat(_path.c_str(), &buf);
  return S_ISDIR(buf.st_mode);
}

bool File::readable() {
  struct stat buf;
  stat(_path.c_str(), &buf);
  return buf.st_mode & S_IRUSR;
}

long int File::size() {
  struct stat buf;
  stat(_path.c_str(), &buf);
  return buf.st_size;
}

std::string File::Read() {
  char buffer[1024];
  std::string data;
  int fd = open(_path.c_str(), O_RDONLY);
  if (fd == -1) throwException("read", "Could not open file");
  int bytes_read = read(fd, buffer, 1024);
  while (bytes_read > 0) {
    data.append(buffer, bytes_read);
    bytes_read = read(fd, buffer, 1024);
  }
  if (close(fd) == -1) throwException("Read", "Could not close file");
  if (bytes_read == -1) throwException("Read", "Could not read file");
  return data;
}

void File::Write(std::string data) {
  int fd = open(_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) throwException("Write", "Could not open file");
  if (write(fd, data.c_str(), data.length()) == -1)
    throwException("Write", "Could not write file");
  if (close(fd) == -1) throwException("Write", "Could not close file");
}

void File::append(std::string data) { (void)data; }

void File::throwException(std::string func, std::string msg) {
  throw std::runtime_error("File::" + func + ": " + msg);
}