#include <unistd.h>  // for the write system call
#include <wait.h>

#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
  pid_t child = fork();

  if (child == -1) {
    return -1;
  }
  if (child == 0) {
    return 1;
  }
  int status;
  sleep(1);
  waitpid(child, &status, 0);
  std::cout << WEXITSTATUS(status) << std::endl;
}
