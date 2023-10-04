#include "AConnection.hpp"

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "Poll.hpp"
#include "global.hpp"

AConnection::AConnection(Address const &serverAddress,
                         Address const &remoteAddress) {
  host = serverAddress;
  client = remoteAddress;
  gettimeofday(&lastTimeActive, NULL);
  bodySize = std::string::npos;
  _writeBufferPos = std::string::npos;
  headDelimiter = "\r\n\r\n";
  pipeIn = -1;
  pipeOut = -1;
  _cgiPid = -1;
  Poll::setTimeout(CONNECTION_TIMEOUT);
}

AConnection::~AConnection() {
  while (_writeStreams.empty() == false) {
    delete _writeStreams.front();
    _writeStreams.pop();
  }
  if (_cgiPid != -1 && Poll::lastForkPid() != 0) {
    int status;
    if (kill(_cgiPid, SIGKILL) == -1) std::cerr << "ERROR: kill()" << std::endl;
    waitpid(_cgiPid, &status, 0);
    accessLog_g.write("reaped CGI process: " + toString<int>(_cgiPid), DEBUG,
                      BLUE);
  }
}

void AConnection::send(std::istream *msg) {
  if (_writeStreams.empty()) Poll::addPollEvent(POLLOUT, this);
  try {
    _writeStreams.push(msg);
  } catch (std::bad_alloc const &) {
    delete msg;
    throw;
  }
}

/**
 * TODO: callback function for POLLERR and POLLHUP?
 */
void AConnection::onPollEvent(struct pollfd &pollfd,
                              CallbackPointer *newCallbackObject,
                              struct pollfd *newPollfd) {
  _newCallbackObject = newCallbackObject;
  _newPollfd = newPollfd;
  if (pollfd.fd == pipeIn) {
    onPipeInPollEvent(pollfd);
    return;
  }
  if (pollfd.fd == pipeOut) {
    onPipeOutPollEvent(pollfd);
    return;
  }
  if (pollfd.revents & POLLERR) throw std::runtime_error("ERROR: POLLERR");
  if (pollfd.revents & POLLHUP) throw std::runtime_error("ERROR: POLLHUP");
  if ((pollfd.revents & (POLLIN | POLLOUT)) == false) {
    onNoPollEvent(pollfd);
    return;
  }
  if (pollfd.revents & POLLIN) onPollIn(pollfd);
  if (pollfd.revents & POLLOUT) onPollOut(pollfd);
  gettimeofday(&lastTimeActive, NULL);
  if (pollfd.events) Poll::setTimeout(CONNECTION_TIMEOUT);
}

void AConnection::onPipeOutPollEvent(struct pollfd &pollfd) {
  if (_cgiPid == -1)
    pollfd.events = 0;
  else {
    if (pollfd.revents & ~POLLOUT) {
      pollfd.events = 0;
      KillCgi();
    } else if (pollfd.revents & POLLOUT) {
      onPipeOutPollOut(pollfd);
    }
  }
  if (pollfd.events == 0) {
    pipeOut = -1;
    _cgiWriteBuffer.clear();  // TODO: unecessary?
  }
}

void AConnection::onPipeOutPollOut(struct pollfd &pollfd) {
  if (_cgiWriteBuffer.empty()) {
    pollfd.events = 0;
    return;
  }
  pollfd.revents &= ~POLLOUT;
  ssize_t lenSent =
      ::write(pipeOut, _cgiWriteBuffer.data(), _cgiWriteBuffer.size());
  if (lenSent == -1) {
    pollfd.events = 0;
    KillCgi();
    return;
  }
  _cgiWriteBuffer.erase(0, lenSent);
  if (_cgiWriteBuffer.empty()) {
    pollfd.events = 0;
  }
}

void AConnection::cgiSend(std::string const &src) { _cgiWriteBuffer += src; }

void AConnection::onPipeInPollEvent(struct pollfd &pollfd) {
  if (_cgiPid == -1)
    pollfd.events = 0;
  else {
    if (pollfd.revents & ~(POLLIN | POLLHUP)) {
      pollfd.events = 0;
      KillCgi();
    } else if (pollfd.revents & (POLLIN | POLLHUP)) {
      onPipeInPollIn(pollfd);
    } else {
      onPipeInNoPollEvent(pollfd);
    }
  }
  if (pollfd.events == 0) {
    pipeIn = -1;
    _cgiReadBuffer.clear();
    Poll::setPollActive(_isListening & POLLIN, this);
  }
}

void AConnection::onPipeInNoPollEvent(struct pollfd &pollfd) {
  struct timeval currentTime;
  struct timeval delta;
  int timeout;

  gettimeofday(&currentTime, NULL);
  delta = currentTime - lastTimeActive;
  timeout = CGI_TIMEOUT - (delta.tv_sec * 1000 + delta.tv_usec / 1000);
  if (timeout <= 0) {
    pollfd.events = 0;
    KillCgi();
    return;
  }
  Poll::setTimeout(timeout);
}

void AConnection::onPipeInPollIn(struct pollfd &pollfd) {
  char tmp[BUFFER_SIZE];

  pollfd.revents &= ~POLLIN;
  ssize_t ret = ::read(pollfd.fd, tmp, BUFFER_SIZE);
  if (ret > 0) {
    try {
      _cgiReadBuffer += std::string(tmp, ret);
      return;
    } catch (...) {
      KillCgi();
      pollfd.events = 0;
      return;
    }
  }

  pollfd.events = 0;
  if (ret == -1) {
    KillCgi();
  } else if (ret == 0) {
    int status;

    waitpid(_cgiPid, &status, 0);  // TODO: error checking?
    pid_t tmp = _cgiPid;
    _cgiPid = -1;
    accessLog_g.write("reaped CGI process: " + toString<int>(tmp), DEBUG, BLUE);
    if (WEXITSTATUS(status) != 0) {
      _cgiReadBuffer.clear();
      _cgiWriteBuffer.clear();
      OnCgiError();
    } else {
      OnCgiRecv(_cgiReadBuffer);
    }
  }
}

void AConnection::KillCgi() {
  int status;

  if (kill(_cgiPid, SIGKILL) == -1) std::cerr << "ERROR: kill()" << std::endl;
  waitpid(_cgiPid, &status, 0);
  pid_t tmp = _cgiPid;
  _cgiPid = -1;
  accessLog_g.write("reaped CGI process: " + toString<int>(tmp), DEBUG, BLUE);
  _cgiReadBuffer.clear();
  _cgiWriteBuffer.clear();
  OnCgiError();
}

void AConnection::onPollOut(struct pollfd &pollfd) {
  std::istream *stream;
  ssize_t lenSent;

  pollfd.revents &= ~POLLOUT;
  if (_writeBufferPos == std::string::npos) {
    stream = _writeStreams.front();
    _writeBufferPos = 0;
    stream->read(_writeBuffer, BUFFER_SIZE);
    _writeBufferSize = stream->gcount();
    if (stream->eof()) {
      delete _writeStreams.front();
      _writeStreams.pop();
    } else if (stream->fail())
      throw std::runtime_error("AConnection::onPollOut(): std::istream.read()");
  }
  lenSent =
      ::send(pollfd.fd, _writeBuffer + _writeBufferPos, _writeBufferSize, 0);
  if (lenSent == -1) {
    throw std::runtime_error(
        std::string("AConnection::onPollOut(): ::send(): ") +
        std::strerror(errno));
  } else if (static_cast<size_t>(lenSent) == _writeBufferSize) {
    _writeBufferPos = std::string::npos;
    if (_writeStreams.empty()) pollfd.events &= ~POLLOUT;
  } else {
    _writeBufferPos += lenSent;
    _writeBufferSize -= lenSent;
  }
}

void AConnection::onPollIn(struct pollfd &pollfd) {
  char tmpbuffer[BUFFER_SIZE];
  ssize_t msglen;

  pollfd.revents &= ~POLLIN;
  msglen = ::recv(pollfd.fd, tmpbuffer, BUFFER_SIZE, 0);
  if (msglen == -1)
    throw std::runtime_error(std::string("AConnection::onPollIn(): ") +
                             std::strerror(errno));
  if (msglen == 0) {
    pollfd.events &= ~POLLIN;
    return;
  }
  _readBuffer += std::string(tmpbuffer, tmpbuffer + msglen);
  passReadBuffer(pollfd);
  if (bodySize == WAIT_FOR_HEAD && _readBuffer.size() > headSizeLimit) {
    pollfd.events = 0;
    pollfd.revents = 0;
    return;
  }
}

/**
 * pollfd.events can be changed by runCgi() and closeConnection()
 */
void AConnection::passReadBuffer(struct pollfd &pollfd) {
  std::string::size_type pos;

  while (pollfd.events & POLLIN) {
    if (bodySize == WAIT_FOR_HEAD) {
      pos = _readBuffer.find(headDelimiter);
      if (pos == std::string::npos) break;
      pos += headDelimiter.size();
      OnHeadRecv(_readBuffer.substr(0, pos));
      _readBuffer.erase(0, pos);
    } else {
      if (_readBuffer.size() < bodySize) break;
      pos = bodySize;
      OnBodyRecv(_readBuffer.substr(0, pos));
      _readBuffer.erase(0, pos);
      // bodySize = WAIT_FOR_HEAD;
    }
  }
}

void AConnection::onNoPollEvent(struct pollfd &) {
  struct timeval currentTime;
  struct timeval delta;
  int timeout;

  gettimeofday(&currentTime, NULL);
  delta = currentTime - lastTimeActive;
  timeout = CONNECTION_TIMEOUT - (delta.tv_sec * 1000 + delta.tv_usec / 1000);
  if (timeout <= 0) {
    std::ostringstream oss;
    oss << client << ": Connection Timed Out";
    throw std::runtime_error(oss.str());
  }
  Poll::setTimeout(timeout);
}

/**
 * CGI
 */
void AConnection::runCGI(std::string const &program,
                         std::vector<std::string> const &arg,
                         std::vector<std::string> const &env) {
  int pipeInArray[2];
  int pipeOutArray[2];

  if (pipe(pipeInArray) == -1) {
    std::cerr << "ERROR: pipe()" << std::endl;
    OnCgiError();
    return;
  }

  if (pipe(pipeOutArray)) {
    std::cerr << "ERROR: pipe()" << std::endl;
    close(pipeInArray[0]);
    close(pipeInArray[1]);
    OnCgiError();
    return;
  }

  _cgiPid = fork();
  Poll::lastForkPid(_cgiPid);
  if (_cgiPid == -1) {
    close(pipeInArray[0]);
    close(pipeInArray[1]);
    close(pipeOutArray[0]);
    close(pipeOutArray[1]);
    std::cerr << "ERROR: fork()" << std::endl;
    OnCgiError();
    return;
  }
  if (_cgiPid == 0) {
    Poll::cleanUp();
    close(pipeInArray[0]);
    close(pipeOutArray[1]);
    if (dup2(pipeInArray[1], STDOUT_FILENO) == -1) {
      close(pipeInArray[1]);
      close(pipeOutArray[0]);
      std::cerr << "ERROR: dup2()" << std::endl;
      exit(EXIT_FAILURE);
    }
    if (dup2(pipeOutArray[0], STDIN_FILENO) == -1) {
      close(pipeOutArray[0]);
      close(pipeInArray[1]);
      std::cerr << "ERROR: dup2()" << std::endl;
      exit(EXIT_FAILURE);
    }
    close(pipeInArray[1]);
    close(pipeOutArray[0]);
    std::vector<char *> c_arg;
    std::vector<char *> c_env;
    c_arg.push_back(const_cast<char *>(program.c_str()));
    for (std::vector<std::string>::const_iterator it = arg.begin();
         it != arg.end(); ++it)
      c_arg.push_back(const_cast<char *>(it->c_str()));
    c_arg.push_back(NULL);
    for (std::vector<std::string>::const_iterator it = env.begin();
         it != env.end(); ++it)
      c_env.push_back(const_cast<char *>(it->c_str()));
    c_env.push_back(NULL);
    execve(program.c_str(), c_arg.data(), c_env.data());
    std::cerr << "webserv: error: execve(): " << program << ": "
              << std::strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
  close(pipeInArray[1]);
  close(pipeOutArray[0]);
  pipeIn = pipeInArray[0];
  pipeOut = pipeOutArray[1];
  _newPollfd[0].fd = pipeIn;
  _newPollfd[0].events = POLLIN;
  _newPollfd[0].revents = 0;
  _newCallbackObject[0].link = true;
  _newCallbackObject[0].ptr = this;
  _newPollfd[1].fd = pipeOut;
  _newPollfd[1].events = POLLOUT;
  _newPollfd[1].revents = 0;
  _newCallbackObject[1].link = true;
  _newCallbackObject[1].ptr = this;
  _isListening = Poll::setPollInactive(this);
  accessLog_g.write("forked CGI process: " + toString<int>(_cgiPid), DEBUG,
                    BLUE);
}

void AConnection::stopReceiving() { Poll::clearPollEvent(POLLIN, this); }
