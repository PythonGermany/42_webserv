#include "ResponsePipe.hpp"

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "Poll.hpp"

ResponsePipe::ResponsePipe(AConnection *callbackObject, int cgiPid)
    : _callbackObject(callbackObject), _cgiPid(cgiPid) {
  gettimeofday(&lastTimeActive, NULL);
  _callbackObjectFlags = Poll::setPollInactive(_callbackObject);
  std::cout << BRIGHT_RED << "ResponsePipe()" << RESET << std::endl;
}

/**
 * undo Poll::setPollInactive, set POLLIN if it was set before POLLINACTIVE
 */
ResponsePipe::~ResponsePipe() {
  int status;

  std::cout << BRIGHT_RED << "~ResponsePipe()" << RESET << std::endl;
  Poll::setPollActive(_callbackObjectFlags & POLLIN, _callbackObject);
  _callbackObject->ResponsePipe = NULL;
  if (_cgiPid == -1 || Poll::lastForkPid() == 0) return;
  kill(_cgiPid, SIGKILL);
  waitpid(_cgiPid, &status, 0);
  _cgiPid = -1;
}

void ResponsePipe::onPollEvent(struct pollfd &pollfd) {
  char tmpbuffer[BUFFER_SIZE];
  int status;

  if ((pollfd.revents & (POLLIN | POLLHUP)) == false) {
    onNoPollEvent(pollfd);
    return;
  }
  pollfd.revents &= ~(POLLIN | POLLHUP);
  ssize_t msglen = ::read(pollfd.fd, tmpbuffer, BUFFER_SIZE);
  if (msglen == -1)
    throw std::runtime_error(std::string("ResponsePipe::onPollEvent(): ") +
                             std::strerror(errno));
  if (msglen == 0) {
    pollfd.events &= ~POLLIN;
    _callbackObject->ResponsePipe = NULL;
    waitpid(_cgiPid, &status, 0);
    _cgiPid = -1;
    try {
      if (WEXITSTATUS(status) != 0)
        _callbackObject->OnCgiError();
      else
        _callbackObject->OnCgiRecv(_readBuffer);
    } catch (...) {
      Poll::remove(_callbackObject);
      throw;
    }
    return;
  }
  _readBuffer += std::string(tmpbuffer, tmpbuffer + msglen);
  gettimeofday(&lastTimeActive, NULL);
  Poll::setTimeout(CGI_TIMEOUT);
  // TODO max cgi message size
}

void ResponsePipe::onNoPollEvent(struct pollfd &pollfd) {
  struct timeval currentTime;
  struct timeval delta;
  int timeout;
  int status;

  gettimeofday(&currentTime, NULL);
  delta = currentTime - lastTimeActive;
  timeout = CGI_TIMEOUT - (delta.tv_sec * 1000 + delta.tv_usec / 1000);

  if (timeout > 0) {
    Poll::setTimeout(timeout);
    return;
  }
  pollfd.events = 0;
  kill(_cgiPid, SIGKILL);
  waitpid(_cgiPid, &status, 0);
  _cgiPid = -1;
  try {
    _callbackObject->OnCgiError();
  } catch (...) {
    _callbackObject->ResponsePipe = NULL;
    Poll::remove(_callbackObject);
    throw;
  }
  return;
}
