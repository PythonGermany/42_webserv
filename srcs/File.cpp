#include "File.hpp"

File::File() {}

File::File(std::string path) { _path = path; }

File::File(const File &rhs) { *this = rhs; }

File &File::operator=(const File &rhs) {
  if (this == &rhs) return *this;
  _path = rhs._path;
  return *this;
}

File::~File() {}

std::string File::getPath() { return _path; }

std::string File::getExtension() {
  std::string::size_type pos = _path.find_last_of('.');
  if (pos == std::string::npos) return "";
  return _path.substr(pos + 1);
}

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

bool File::writable() {
  struct stat buf;
  stat(_path.c_str(), &buf);
  return buf.st_mode & S_IWUSR;
}

long int File::size() {
  struct stat buf;
  stat(_path.c_str(), &buf);
  return buf.st_size;
}

std::string File::Read() {
  std::string data;
  int fd = open(_path.c_str(), O_RDONLY);

  if (fd == -1) throwException("read", "Could not open file");
  while (true) {
    char buffer[1024];
    int bytes_read = read(fd, buffer, 1024);
    if (bytes_read < 0) throwException("Read", "Could not read file");
    data += std::string(buffer, bytes_read);
    if (bytes_read < 1024) break;
  }
  if (close(fd) == -1) throwException("Read", "Could not close file");
  return data;
}

void File::Write(std::string data, bool append) {
  int fd = open(_path.c_str(),
                O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
  if (fd == -1) throwException("Write", "Could not open file");
  if (write(fd, data.c_str(), data.length()) == -1)
    throwException("Write", "Could not write file");
  if (close(fd) == -1) throwException("Write", "Could not close file");
}

void File::throwException(std::string func, std::string msg) {
  throw std::runtime_error("File: " + func + ": " + msg);
}