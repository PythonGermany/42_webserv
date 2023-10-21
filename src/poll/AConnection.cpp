#include "AConnection.hpp"

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "Poll.hpp"
#include "global.hpp"

int AConnection::_connectionTimeout = CONNECTION_TIMEOUT;

int AConnection::_cgiTimout = CGI_TIMEOUT;

AConnection::AConnection(Address const &serverAddress,
                         Address const &remoteAddress) {
  host = serverAddress;
  client = remoteAddress;
  gettimeofday(&lastTimeActive, NULL);
  bodySize = std::string::npos;
  _writeBufferPos = std::string::npos;
  pipeIn = -1;
  pipeOut = -1;
  _cgiPid = -1;
  Poll::setTimeout(_connectionTimeout);
}

AConnection::~AConnection() {
  while (_writeStreams.empty() == false) {
    delete _writeStreams.front();
    _writeStreams.pop();
  }
  if (_cgiPid != -1 && Poll::lastForkPid() != 0) {
    int status;
    kill(_cgiPid, SIGKILL);
    waitpid(_cgiPid, &status, 0);
    try {
      accessLog_g.write(
          "reaped CGI process: " + toString<int>(_cgiPid) + " exit status: " +
              toString<int>(WEXITSTATUS(status)) + " reason: object destructed",
          DEBUG, BLUE);
    } catch (...) {
    }
  }
}

void AConnection::initConnectionTimeout(int connectionTimeout) {
  _connectionTimeout = connectionTimeout;
}

void AConnection::initCgiTimout(int cgiTimout) { _cgiTimout = cgiTimout; }

void AConnection::setReadState(state_t readState) {
  _readState = readState;

  switch (_readState) {
    case REQUEST_LINE:
      readDelimiter = "\r\n";
      break;
    case HEAD:
      readDelimiter = "\r\n\r\n";
      break;
    case BODY:
      readDelimiter = "";
      break;
    case CHUNK_SIZE:
      readDelimiter = "\r\n";
      break;
    case TRAILER:
      readDelimiter = "\r\n";
      break;

    default:
      readDelimiter = "\r\n";
      break;
  }
}

std::string const &AConnection::getReadDelimiter() const {
  return readDelimiter;
}

void AConnection::send(std::istream *msg) {
  if (msg == NULL) return;
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
  if (pollfd.events) Poll::setTimeout(_connectionTimeout);
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

void AConnection::cgiCloseSendPipe() {
  if (pipeOut == -1) return;
  if (_newPollfd[0].fd == pipeOut)
    _newPollfd[0].events &= ~POLLINACTIVE;
  else if (_newPollfd[1].fd == pipeOut)
    _newPollfd[1].events &= ~POLLINACTIVE;
  else
    Poll::clearPollEvent(POLLINACTIVE, pipeOut);
  _isListening = Poll::setPollInactive(this);
}

void AConnection::onPipeOutPollOut(struct pollfd &pollfd) {
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
    pollfd.events &= ~POLLOUT;
  }
}

void AConnection::cgiSend(std::string const &src) {
  if (pipeOut == -1) {
    errorLog_g.write("cgiSend() was called without an active pipe", ERROR);
    return;
  }
  if (_cgiWriteBuffer.empty()) {
    if (_newPollfd[0].fd == pipeOut)
      _newPollfd[0].events |= POLLOUT;
    else if (_newPollfd[1].fd == pipeOut)
      _newPollfd[1].events |= POLLOUT;
    else
      Poll::addPollEvent(POLLOUT, pipeOut);
  }
  accessLog_g.write("CGI: in '" + src + "'", VERBOSE);
  _cgiWriteBuffer += src;
}

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
    if (_isListening) Poll::setPollActive(_isListening, this);
  }
}

void AConnection::onPipeInNoPollEvent(struct pollfd &pollfd) {
  struct timeval currentTime;
  struct timeval delta;
  int timeout;

  gettimeofday(&currentTime, NULL);
  delta = currentTime - lastTimeActive;
  timeout = _cgiTimout - (delta.tv_sec * 1000 + delta.tv_usec / 1000);
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
      _cgiReadBuffer.append(tmp, ret);
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
    accessLog_g.write(
        "reaped CGI process: " + toString<int>(tmp) +
            " exit status: " + toString<int>(WEXITSTATUS(status)) +
            " reason: process closed stdout",
        DEBUG, BLUE);
    if (WEXITSTATUS(status) != 0 && WEXITSTATUS(status) != 1) {
      errorLog_g.write("ignore output (crash): \n" + _cgiReadBuffer, VERBOSE,
                       RED);
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

  kill(_cgiPid, SIGKILL);
  waitpid(_cgiPid, &status, 0);
  pid_t tmp = _cgiPid;
  _cgiPid = -1;
  accessLog_g.write("reaped CGI process: " + toString<int>(tmp) +
                        " exit status: " + toString<int>(WEXITSTATUS(status)) +
                        " reason: process killed",
                    DEBUG, BLUE);
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
  _readBuffer.append(tmpbuffer, msglen);
  passReadBuffer(pollfd);
  if (pollfd.events & POLLIN &&
      (_readState == REQUEST_LINE || _readState == HEAD ||
       _readState == TRAILER) &&
      _readBuffer.size() > headSizeLimit) {
    pollfd.events = 0;
    pollfd.revents = 0;
  }
}

/**
 * pollfd.events can be changed by runCgi() and closeConnection()
 */
void AConnection::passReadBuffer(struct pollfd &pollfd) {
  std::string::size_type pos;

  while (pollfd.events & POLLIN) {
    pos = _readBuffer.find(readDelimiter);
    if (pos == std::string::npos) break;
    size_t tmpReadDelim = readDelimiter.size();

    switch (_readState) {
      case REQUEST_LINE:
        OnRequestRecv(_readBuffer.substr(0, pos));
        break;
      case HEAD:
        OnHeadRecv(_readBuffer.substr(0, pos));
        break;
      case CHUNK_SIZE:
        OnChunkSizeRecv(_readBuffer.substr(0, pos));
        break;
      case TRAILER:
        OnTrailerRecv(_readBuffer.substr(0, pos));
        break;
      case BODY:
        if (_readBuffer.size() < bodySize) return;
        pos = bodySize;
        OnBodyRecv(_readBuffer.substr(0, pos));
        break;

      default:
        throw std::runtime_error("passReadBuffer(): Undefined read state");
    }
    _readBuffer.erase(0, pos + tmpReadDelim);
  }
}

void AConnection::onNoPollEvent(struct pollfd &) {
  struct timeval currentTime;
  struct timeval delta;
  int timeout;

  gettimeofday(&currentTime, NULL);
  delta = currentTime - lastTimeActive;
  timeout = _connectionTimeout - (delta.tv_sec * 1000 + delta.tv_usec / 1000);
  if (timeout <= 0) {
    std::ostringstream oss;
    oss << client << ": Connection Timed Out";
    throw std::runtime_error(oss.str());
  }
  Poll::setTimeout(timeout);
}

static bool initPipes(int a[2], int b[2]) {
  static int const fdsize = 4;
  int fd[fdsize];
  // int flags;

  if (pipe(&(fd[0])) == -1) return false;
  if (pipe(&(fd[2])) == -1) {
    close(fd[0]);
    close(fd[1]);
    return false;
  }
  // for (int i = 0; i < fdsize; ++i) { // TODO: Figure out why this breaks
  // wordpress
  //   flags = fcntl(fd[i], F_GETFL, 0);
  //   if (flags == -1 || fcntl(fd[i], F_SETFL, flags | O_NONBLOCK) == -1) {
  //     close(fd[0]);
  //     close(fd[1]);
  //     close(fd[2]);
  //     close(fd[3]);
  //     return false;
  //   }
  // }
  a[0] = fd[0];
  a[1] = fd[1];
  b[0] = fd[2];
  b[1] = fd[3];
  return true;
}

/**
 * The http object is destroyed after calling fork(). The parameters must still
 * remain valid. program cannot be a reference to Http::cgiProgramPathname
 * because this is a member variable of the Http object, which is destroyed.
 */
void AConnection::runCGI(std::string program,
                         std::vector<std::string> const &arg,
                         std::vector<std::string> const &env) {
  int pipeInArray[2];
  int pipeOutArray[2];

  if (initPipes(pipeInArray, pipeOutArray) == false) {
    errorLog_g.write(std::string("initPipes(): ") + std::strerror(errno),
                     ERROR);
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
    errorLog_g.write("ERROR: fork()", DEBUG, BRIGHT_RED);
    OnCgiError();
    return;
  }
  if (_cgiPid == 0) {
    Log::setLogToTerminal(false, true);
    accessLog_g.setInitialized(false);
    errorLog_g.setInitialized(false);
    Poll::cleanUp();
    close(pipeInArray[0]);
    close(pipeOutArray[1]);

    if (chdir(File(arg[0]).getDir().c_str()) == -1 ||
        dup2(pipeInArray[1], STDOUT_FILENO) == -1 ||
        dup2(pipeOutArray[0], STDIN_FILENO) == -1) {
      close(pipeInArray[1]);
      close(pipeOutArray[0]);
      errorLog_g.write("ERROR: dup2()", DEBUG, BRIGHT_RED);
      exit(2);
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
    std::cerr << "webserv: " << BRIGHT_RED << "error: execve(): " << program
              << ": " << std::strerror(errno) << RESET << std::endl;
    exit(2);
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
  _newPollfd[1].events = POLLINACTIVE;
  _newPollfd[1].revents = 0;
  _newCallbackObject[1].link = true;
  _newCallbackObject[1].ptr = this;
  _isListening = 0;
  accessLog_g.write("forked CGI process: " + toString<int>(_cgiPid), DEBUG,
                    BLUE);
}

void AConnection::stopReceiving() { Poll::clearPollEvent(POLLIN, this); }
