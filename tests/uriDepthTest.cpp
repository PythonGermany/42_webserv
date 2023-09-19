#include <iostream>

#include "Uri.hpp"

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    Uri uri(argv[i]);
    std::cout << argv[i]
              << " is out of bounds: " << (uri.pathOutOfBound() ? "yes" : "no")
              << std::endl;
  }
  return 0;
}