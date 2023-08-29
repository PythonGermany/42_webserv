#include <iostream>
#include "Cgi.hpp"
#include "File.hpp"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <file>" << std::endl;
    return 1;
  }
  File file(argv[1]);
  Cgi cgi;

  cgi.addEntry("php", "/usr/bin/php-cgi");
  if (cgi.isCgiFile(file))
    std::cout << cgi.executeFile(file, "/home/pythongermany/_Projects/42_webserv") << std::endl;
  else
    std::cout << "Not a cgi file" << std::endl;
  return 0;
}