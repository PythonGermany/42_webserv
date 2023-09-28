#include "RequestPipe.hpp"

#include <iostream>

#include "Poll.hpp"

RequestPipe::RequestPipe(AConnection *src) {
  callbackObject = src;
  std::cout << BRIGHT_RED << "RequestPipe()" << RESET << std::endl;
}

RequestPipe::~RequestPipe() {
  callbackObject->RequestPipe = NULL;
  std::cout << BRIGHT_RED << "~RequestPipe()" << RESET << std::endl;
}

void RequestPipe::onPollEvent(struct pollfd &pollfd) try {
  callbackObject->onPipePollOut(pollfd);
} catch (...) {
  try {
    callbackObject->OnCgiError();
  } catch (...) {
    callbackObject->RequestPipe = NULL;
    Poll::remove(callbackObject);
    throw;
  }
}
