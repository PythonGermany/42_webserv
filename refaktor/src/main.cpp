#include <fcntl.h>

#include <iostream>

#include "ListenSocket.hpp"
#include "Poll.hpp"

// void *operator new(size_t size) throw(std::bad_alloc) {
//   std::cerr << "new" << std::endl;
//   if (rand() % 25 == 0) throw std::bad_alloc();
//   return malloc(size);
// }

// void operator delete(void *p) throw() { free(p); }

#include <dirent.h>
size_t count_fds() {
  DIR *dir_ptr = NULL;
  struct dirent *dirent_ptr = NULL;
  std::stringstream ss;
  size_t count = 0;

  ss << "/proc/" << static_cast<int>(getpid()) << "/fd/";

  dir_ptr = opendir(ss.str().c_str());
  if (dir_ptr == NULL) throw std::runtime_error("ERROR: opendir()");

  while ((dirent_ptr = readdir(dir_ptr))) {
    ++count;
  }
  closedir(dir_ptr);
  return count;
}

int main() {
  size_t fds = count_fds();
  std::set<Address> addr = Address::resolveHost("8080");
  std::set<Address>::const_iterator it = addr.begin();

  while (it != addr.end()) {
    ListenSocket::create(*it);
    std::cout << "listen on: " << *it << std::endl;
    ++it;
  }
  while (true) {
    if (Poll::poll() == false) break;
  }
  Poll::cleanUp();
  if (fds != count_fds()) throw std::runtime_error("unclosed fd");
}
