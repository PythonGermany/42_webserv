#include "File.hpp"

File::File() {}

File::File(std::string path) { _path = path; }

File::File(const File &rhs) { *this = rhs; }

File &File::operator=(const File &rhs) {
  _path = rhs._path;
  return *this;
}

File::~File() {}

std::string File::getPath() const { return _path; }

std::string File::getDir() const {
  std::string::size_type pos = _path.find_last_of('/');
  if (pos == std::string::npos) return "";
  return _path.substr(0, pos + 1);
}

std::string File::getName() const {
  std::string::size_type pos = _path.find_last_of("/");
  if (pos == std::string::npos) return _path;
  return _path.substr(pos + 1);
}

std::string File::getExtension() const {
  std::string name = getName();
  std::string::size_type pos = name.find_last_of('.');
  if (pos == std::string::npos) return "";
  return name.substr(pos + 1);
}

void File::setPath(std::string path) { _path = path; }

std::vector<std::string> File::list(std::string path) {
  std::vector<std::string> files;
  DIR *dir = opendir(path.c_str());
  struct dirent *ent;
  if (dir != NULL) {
    ent = readdir(dir);
    while (ent != NULL) {
      files.push_back(ent->d_name);
      ent = readdir(dir);
    }
    closedir(dir);
  } else
    throw std::runtime_error("File: list: Could not open directory: " +
                             std::string(strerror(errno)) + " " + path);
  return files;
}

bool File::exists() const {
  struct stat buf;
  return stat(_path.c_str(), &buf) == 0;
}

bool File::file() const {
  struct stat buf;
  if (stat(_path.c_str(), &buf) == -1) return false;
  return S_ISREG(buf.st_mode);
}

bool File::isSymLink() const {
  struct stat buf;
  if (lstat(_path.c_str(), &buf) == -1) return false;
  return S_ISLNK(buf.st_mode);
}

bool File::dir() const {
  struct stat buf;
  if (stat(_path.c_str(), &buf) == -1) return false;
  return S_ISDIR(buf.st_mode);
}

bool File::readable() const {
  struct stat buf;
  if (stat(_path.c_str(), &buf) == -1) return false;
  return buf.st_mode & S_IRUSR;
}

bool File::writable() const {
  struct stat buf;
  if (stat(_path.c_str(), &buf) == -1) return false;
  return buf.st_mode & S_IWUSR;
}

size_t File::size() const {  // TODO: Check for errors when used
  struct stat buf;
  if (stat(_path.c_str(), &buf) == -1) return -1;
  return buf.st_size;
}

time_t File::getLastModified() const {
  struct stat buf;
  if (stat(_path.c_str(), &buf) == -1) return 0;
  return buf.st_mtime;
}

std::string File::lastModified(std::string format) const {
  time_t buf = getLastModified();
  return getTime(format, &buf);
}

int File::resolveSymlink() {
  char resolvedPath[PATH_MAX];
  if (realpath(_path.c_str(), resolvedPath) == NULL) return -1;
  _path = std::string(resolvedPath);
  return 0;
}