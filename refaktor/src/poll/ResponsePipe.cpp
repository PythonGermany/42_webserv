#include "ResponsePipe.hpp"

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "AConnection.hpp"
#include "Poll.hpp"

ResponsePipe::ResponsePipe() : _callbackObject(NULL), _cgiPid(-1), _fd(-1) {}

ResponsePipe::ResponsePipe(AConnection *callbackObject, int cgiPid, int fd)
    : _callbackObject(callbackObject), _cgiPid(cgiPid), _fd(fd) {
  gettimeofday(&lastTimeActive, NULL);
  _callbackObjectFlags = Poll::setPollInactive(_callbackObject);
  std::cout << "ResponsePipe()" << std::endl;
}

ResponsePipe::ResponsePipe(ResponsePipe const &other) { *this = other; }

ResponsePipe &ResponsePipe::operator=(ResponsePipe const &other) {
  _callbackObject = other._callbackObject;
  _callbackObjectFlags = other._callbackObjectFlags;
  _readBuffer = other._readBuffer;
  lastTimeActive = other.lastTimeActive;
  _cgiPid = other._cgiPid;
  _fd = other._fd;
  return *this;
}

/**
 * undo Poll::setPollInactive, set POLLIN if it was set before POLLINACTIVE
 */
ResponsePipe::~ResponsePipe() {
  int status;

  std::cout << "~ResponsePipe()" << std::endl;
  if (_cgiPid == -1 || Poll::lastForkPid() == 0) return;
  kill(_cgiPid, SIGKILL);
  waitpid(_cgiPid, &status, 0);
  _cgiPid = -1;
}

void ResponsePipe::onPollEvent(struct pollfd &pollfd,
                               CallbackPointer *newCallbackObject,
                               struct pollfd *newPollfd) {
  char tmpbuffer[BUFFER_SIZE];
  int status;

  (void)newCallbackObject;
  (void)newPollfd;
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
    waitpid(_cgiPid, &status, 0);
    _cgiPid = -1;
    if (WEXITSTATUS(status) != 0)
      _callbackObject->OnCgiError();
    else
      _callbackObject->OnCgiRecv(_readBuffer);
    Poll::setPollActive(_callbackObjectFlags, _callbackObject);
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
  _callbackObject->OnCgiError();
  Poll::setPollActive(_callbackObjectFlags, _callbackObject);
  return;
}

int ResponsePipe::getFd() const { return _fd; }

short ResponsePipe::getFlags() const { return _callbackObjectFlags; }
