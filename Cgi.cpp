#include "Cgi.hpp"

Cgi::Cgi() {}

Cgi::~Cgi() {}

void Cgi::addEntry(std::string key, std::string value) { _entries[key] = value; }

std::string Cgi::getEntry(std::string key) {
  if (_entries.find(key) == _entries.end())
    throwException("getEntry", "key not found");
  return _entries[key];
}

std::map<std::string, std::string> Cgi::getEntries() { return _entries; }

bool Cgi::isCgiFile(File &file) {
  std::string extension = file.getExtension();
  return _entries.find(extension) != _entries.end();
}

std::string Cgi::executeFile(File &file, std::string pwd) {
  std::string data;
  std::string path = file.getPath();
  std::string exec = getEntry(file.getExtension());
  int fd[2];

  if (pipe(fd) == -1) throwException("executeFile", "pipe error");
  int fork_ret = fork();
  if (fork_ret == -1) throwException("executeFile", "fork error");
  if (fork_ret == 0) {
    if (chdir(pwd.c_str()) == -1) throwException("executeFile", "chdir error");
    if (dup2(fd[1], STDOUT_FILENO) == -1)
      throwException("executeFile", "dup2 error");
    int error_fd = open("webserv_cgi_error.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (error_fd == -1) throwException("executeFile", "open error");
    if (dup2(error_fd, STDERR_FILENO) == -1)
      throwException("executeFile", "dup2 error");
    if (close(fd[0]) == -1) throwException("executeFile", "close error");
    if (close(fd[1]) == -1) throwException("executeFile", "close error");
    if (execl(exec.c_str(), exec.c_str(), path.c_str(), NULL) == -1)
      throwException("executeFile", "execl error");
  } else {
    if (close(fd[1]) == -1) throwException("executeFile", "close error");
    while (true) {
      char buffer[1024];
      int bytes_read = read(fd[0], buffer, 1024);
      if (bytes_read < 0) throwException("executeFile", "read error");
      data += std::string(buffer, bytes_read);
      if (bytes_read < 1024) break;
    }
    if (close(fd[0]) == -1) throwException("executeFile", "close error");
  } 
  return data;
}

void Cgi::throwException(std::string func, std::string msg) {
  throw std::runtime_error("Cgi::" + func + ": " + msg);
}