#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "Request.hpp"
#include "colors.hpp"

typedef struct request_s {
  std::string method;
  std::string uri;
  std::string version;
  std::string responseCode;
} request_t;

const request_t tests_g[] = {
    {"GET", "/", "HTTP/2", "505"},
    {"GET", "/", "HTTP/1.1", "200"},
    {"GET", "/*", "HTTP/1.1", "404"},
    {"GET", "*/", "HTTP/1.1", "400"},
    {"OPTIONS", "*", "HTTP/1.1", "200"},
    {"OPTIONS", "/*", "HTTP/1.1", "200"},
    {"OPTIONS", "*/", "HTTP/1.1", "400"},
    {"OPTIONS", "../", "HTTP/1.1", "400"},
    {"GET", "/../", "HTTP/1.1", "400"},
    {"GET", "/te4st/../", "HTTP/1.1", "200"},
    {"GET", "/test/../../", "HTTP/1.1", "400"},
    {"LOL", "/", "HTTP/1.1", "501"},
    {"GET", "http://localhost/", "HTTP/1.1", "200"},
    {"PUT", "/", "HTTP/1.1", "405"},
    {"POST", "/", "HTTP/1.1", "405"},
    {"PUT", "/upload/test.txt", "HTTP/1.1", "411"},
    {"POST", "/listing/cgi/index.php", "HTTP/1.1", "411"},
    {"GET", "/redirect/", "HTTP/1.1", "301"},
    {"GET", "/redirect2/", "HTTP/1.1", "301"}};

int readStatusLine(int fd, int i, std::string requestLine) {
  std::string data;
  char buffer[1000];
  int ret = read(fd, buffer, 1000);
  if (ret == 0) return 1;
  data.append(buffer, ret);
  data = data.erase(data.find("\r\n"));
  std::vector<std::string> statusLine =
      split<std::vector<std::string> >(data, " ", true);
  if (statusLine[1] == tests_g[i].responseCode)
    std::cout << GREEN << "Success: " << requestLine
              << " - > Got : " << tests_g[i].responseCode << RESET << std::endl;
  else
    std::cout << RED << "Fail: " << requestLine
              << " - > Expected: " << tests_g[i].responseCode
              << " Got: " << statusLine[1] << RESET << std::endl;
  return 0;
}

int main(int argc, char **argv) {
  for (size_t i = 0; i < sizeof(tests_g) / sizeof(request_t); i++) {
    int fd;
    struct sockaddr_in addr;
    struct sockaddr_in cmp;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      std::cerr << "error: socket\n";
      return 1;
    }
    if (connect(fd, (sockaddr *)(&addr), sizeof(addr)) == -1) {
      std::cerr << "error: connect\n";
      return 1;
    }
    std::string requestLine =
        tests_g[i].method + " " + tests_g[i].uri + " " + tests_g[i].version;
    std::string msg =
        requestLine + "\r\n" + "Host: localhost:8080\r\n" + "\r\n";
    write(fd, msg.c_str(), msg.size());
    readStatusLine(fd, i, requestLine);
    close(fd);
  }
}