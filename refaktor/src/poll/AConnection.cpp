#include "AConnection.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <wait.h>

#include <iostream>

#include "Poll.hpp"

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
    std::cerr << "waitpid()" << std::endl;
    _cgiPid = -1;
  }
  // if (ResponsePipe != NULL) Poll::remove(ResponsePipe);
  // if (RequestPipe != NULL) Poll::remove(RequestPipe);
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

void AConnection::onPollEvent(struct pollfd &pollfd,
                              CallbackPointer *newCallbackObject,
                              struct pollfd *newPollfd) {
  _newCallbackObject = newCallbackObject;
  _newPollfd = newPollfd;
  if (pollfd.fd == pipeIn) {
    std::cerr << "i am a link" << std::endl;
    if (_cgiPid == -1) {  // was stop by other pipe
      pollfd.events = 0;
      pollfd.revents = 0;
    }
    onPipeInPollEvent(pollfd);
    if (pollfd.events == 0) {
      pipeIn = -1;
      _cgiReadBuffer.clear();
      Poll::setPollActive(_isListening & POLLIN, this);
    }
    return;
  }
  if (pollfd.fd == pipeOut) {
    if (_cgiPid == -1) {  // was stop by other pipe
      pollfd.events = 0;
      pollfd.revents = 0;
    }
    onPipeOutPollEvent(pollfd);
    if (pollfd.events == 0) {
      pipeOut = -1;
      _cgiWriteBuffer.clear();
    }
  }
  std::cerr << "i am not a link" << std::endl;
  if (!(pollfd.revents & POLLIN || pollfd.revents & POLLOUT)) {
    onNoPollEvent(pollfd);
    return;
  }
  if (pollfd.revents & POLLIN) onPollIn(pollfd);
  if (pollfd.revents & POLLOUT) onPollOut(pollfd);
  gettimeofday(&lastTimeActive, NULL);
}

void AConnection::onPipeOutPollEvent(struct pollfd &pollfd) {
  if (_cgiWriteBuffer.empty()) {
    pollfd.events = 0;
    return;
  }
  if (pollfd.revents == false) return;
  if (pollfd.revents & ~(POLLOUT)) {
    pollfd.revents = 0;
    pollfd.events = 0;
    KillCgi();
    return;
  }
  pollfd.revents = 0;
  ssize_t lenSent =
      ::write(pipeOut, _cgiWriteBuffer.data(), _cgiWriteBuffer.size());
  if (lenSent != -1) {
    pollfd.events = 0;
    KillCgi();
    return;
  }
  try {
    _cgiWriteBuffer.erase(0, lenSent);
  } catch (std::bad_alloc const &e) {
    pollfd.events = 0;
    KillCgi();
    return;
  }
  if (_cgiWriteBuffer.empty()) {
    pollfd.events = 0;
  }
}

void AConnection::cgiSend(std::string const &src) { _cgiWriteBuffer += src; }

void AConnection::onPipeInPollEvent(struct pollfd &pollfd) {
  char tmp[BUFFER_SIZE];

  if (pollfd.revents == false) return;
  std::cerr << "check pipe" << std::endl;
  if (pollfd.revents & ~(POLLHUP | POLLIN)) {
    pollfd.revents = 0;
    pollfd.events = 0;
    KillCgi();
    return;
  }
  pollfd.revents = 0;
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
    _cgiPid = -1;
    std::cerr << "waitpid()" << std::endl;
    if (WEXITSTATUS(status) != 0) {
      _cgiReadBuffer.clear();
      _cgiWriteBuffer.clear();
      OnCgiError();
    } else {
      OnCgiRecv(_cgiReadBuffer);
    }
    // waitforchild normal
  }
}

void AConnection::KillCgi() {
  int status;

  if (kill(_cgiPid, SIGKILL) == -1) std::cerr << "ERROR: kill()" << std::endl;
  waitpid(_cgiPid, &status, 0);
  std::cerr << "waitpid()" << std::endl;
  _cgiPid = -1;
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
  if (bodySize == std::string::npos && _readBuffer.size() > headSizeLimit) {
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
      OnBodyRecv(_readBuffer.substr(0, bodySize));
      _readBuffer.erase(0, bodySize);
      bodySize = WAIT_FOR_HEAD;
    }
  }
}

void AConnection::onNoPollEvent(struct pollfd &pollfd) {
  struct timeval currentTime;
  struct timeval delta;
  int timeout;

  if (pollfd.events & POLLINACTIVE) return;
  gettimeofday(&currentTime, NULL);
  delta = currentTime - lastTimeActive;
  timeout = CONNECTION_TIMEOUT - (delta.tv_sec * 1000 + delta.tv_usec / 1000);
  if (timeout <= 0) {
    std::ostringstream oss;
    oss << client;
    throw std::runtime_error(oss.str() + ": Connection Timed Out");
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

  if (pipe(pipeOutArray) == -1) {
    std::cerr << "ERROR: pipe()" << std::endl;
    close(pipeInArray[0]);
    close(pipeInArray[0]);
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
    std::cerr << "webserv: error: execve(): " << std::strerror(errno)
              << std::endl;
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
  std::cerr << "create link" << std::endl;
}

// void AConnection::cgiSend(std::string msg) { _cgiWriteBuffer += msg; }

// void AConnection::onPipePollOut(struct pollfd &pollfd) {
//   size_t lenToSend;
//   ssize_t lenSent;

//   if (pollfd.revents & POLLHUP) {
//     pollfd.events = 0;
//     return;
//   }
//   pollfd.revents &= ~POLLOUT;
//   if (_cgiWriteBuffer.size() > BUFFER_SIZE)
//     lenToSend = BUFFER_SIZE;
//   else
//     lenToSend = _cgiWriteBuffer.size();
//   lenSent = ::write(pollfd.fd, _cgiWriteBuffer.data(), lenToSend);
//   if (lenSent == -1)
//     std::runtime_error(std::string("AConnection::onPipePollOut(): ") +
//                        std::strerror(errno));
//   _cgiWriteBuffer.erase(0, lenSent);
//   if (_cgiWriteBuffer.empty()) {
//     pollfd.events &= ~POLLOUT;
//   }
// }

// void AConnection::closeConnection() { Poll::clearPollEvent(POLLIN, this); }
