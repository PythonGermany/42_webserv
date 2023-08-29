#include <iostream>

#include "File.hpp"

int main() {
  File file("test.txt");

  std::cout << "exists: " << file.exists() << std::endl;
  std::cout << "dir: " << file.dir() << std::endl;
  std::cout << "readable: " << file.readable() << std::endl;
  file.Write("Hello world!");
  std::cout << "exists: " << file.exists() << std::endl;
  std::cout << "dir: " << file.dir() << std::endl;
  std::cout << "readable: " << file.readable() << std::endl;
  return 0;
}