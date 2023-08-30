#include "File.hpp"

int File::_filesOpen = 0;

File::File() : _fd(-1) {}

File::File(std::string path) : _fd(-1) { _path = path; }

File::File(const File &rhs) { *this = rhs; }

File &File::operator=(const File &rhs) {
  if (this == &rhs) return *this;
  _fd = rhs._fd;
  _path = rhs._path;
  return *this;
}

File::~File() {}

int File::getFilesOpen() { return _filesOpen; }

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

bool File::isOpen() { return _fd != -1; }

void File::create() {
  if (exists()) return;
  size_t pos = _path.find_first_of('/');
  while (pos != std::string::npos) {
    std::string dir = _path.substr(0, pos);
    if (dir != "" && !File(dir).exists()) {
      if (mkdir(dir.c_str(), 0755) == -1)
        throwException("create", "Could not create directory: " +
                                     std::string(strerror(errno)) + " " +
                                     _path);
    }
    pos = _path.find_first_of('/', pos + 1);
  }
  open(O_WRONLY | O_CREAT);
  close();
}

void File::open(int flags, mode_t mode) {
  if (_fd != -1) return;
  _fd = ::open(_path.c_str(), flags, mode);
  if (_fd == -1)
    throwException("open", "Could not open file: " +
                               std::string(strerror(errno)) + " " + _path);
  _filesOpen++;
}

void File::close() {
  if (_fd == -1) return;
  if (::close(_fd) == -1)
    throwException("close", "Could not close file: " +
                                std::string(strerror(errno)) + " " + _path);
  _fd = -1;
  _filesOpen--;
}

std::string File::read() {
  std::string data;

  while (true) {
    char buffer[1024];
    int bytes_read = ::read(_fd, buffer, 1024);
    if (bytes_read < 0) throwException("read", "Failed to read file");
    data += std::string(buffer, bytes_read);
    if (bytes_read < 1024) break;
  }
  return data;
}

void File::write(std::string data) {
  if (::write(_fd, data.c_str(), data.length()) == -1)
    throwException("write", "Could not write file");
}

void File::throwException(std::string func, std::string msg) {
  throw std::runtime_error("File: " + func + ": " + msg);
}