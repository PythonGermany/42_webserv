#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  std::string type;
  std::string in;
  std::string out;

  if (argc < 3) {
    std::cout << "error: missing file operand" << std::endl;
    std::cout << "usage: " << argv[0] << " [r|j] [file ...]" << std::endl;
    return 1;
  }
  type = argv[1];
  if (type == "r") {
    in = "	";
    out = "  ";
  } else if (type == "j") {
    in = "  ";
    out = "	";
  } else {
    std::cout << "error: invalid type" << std::endl;
    std::cout << "usage: " << argv[0] << " [r|j] [file ...]" << std::endl;
    return 1;
  }
  for (int i = 2; i < argc; i++) {
    std::fstream file(argv[i], std::fstream::in);
    if (!file.is_open()) {
      std::cout << "error: " << argv[i] << ": " << strerror(errno) << std::endl;
      continue;
    }
    std::string line;
    std::string output;
    size_t pos;
    while (std::getline(file, line)) {
      pos = 0;
      while (line.find(in) == pos) {
        line.replace(pos, in.length(), out);
        pos += out.length();
      }
      output += line + "\n";
    }
    file.close();
    file.open(argv[i], std::fstream::out | std::fstream::trunc);
    if (!file.is_open()) {
      std::cout << "error: " << argv[i] << ": " << strerror(errno) << std::endl;
      continue;
    }
    file << output;
  }
  return 0;
}