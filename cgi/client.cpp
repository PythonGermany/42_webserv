#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <vector>

int main(int, char **, char **) {
  int fd;

  struct sockaddr_in addr;
  struct sockaddr_in cmp;

  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(8080);
  addr.sin_family = AF_INET;
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    std::cerr << "error: socket\n";
  }
  if (connect(fd, (sockaddr *)(&addr), sizeof(addr)) == -1) {
    std::cerr << "error: connect\n";
  }
  std::cout << "connected to: 127.0.0.1:8080" << std::endl;
  std::string msg;
  msg += "POST /dump.php HTTP/1.1\r\n";
  msg += "Host: 127.0.0.1\r\n";
  msg += "Transfer-Encoding: chunked\r\n";
  msg += "Content-Type: application/x-www-form-urlencoded\r\n";
  msg += "\r\n";

  std::vector<std::string> body;
  body.push_back("hello=world");
  body.push_back("&bla=blup");
  body.push_back("&newline=some text with a newline\r\nin the middle");
  // body.push_back("does it\r\n");
  // body.push_back("work ???????????????????????????????????");

  write(fd, msg.c_str(), msg.size());

  for (size_t i = 0; i < body.size(); ++i) {
    std::stringstream ss;
    ss << std::hex << body[i].size() << "\r\n";
    std::string size(ss.str());
    write(fd, size.c_str(), size.size());
    write(fd, body[i].c_str(), body[i].size());
    write(fd, "\r\n", 2);
  }
  write(fd, "0\r\n\r\n", 5);
  char c;
  while (read(fd, &c, 1) == 1) {
    std::cout << c;
    std::cout.flush();
  }
  close(fd);
  return 0;
}