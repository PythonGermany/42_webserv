#include <fcntl.h>

#include <iostream>

#include "ListenSocket.hpp"
#include "Poll.hpp"

void *operator new(size_t size) throw(std::bad_alloc) {
  //   if (rand() % 25 == 0) throw std::bad_alloc();
  return malloc(size);
}

void operator delete(void *p) throw() { free(p); }

int main() {
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
  std::cout << "no open files: " << (3 == open("Makefile", O_RDONLY))
            << std::endl;
}
