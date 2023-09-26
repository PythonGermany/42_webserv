#include "AConnection.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include "Poll.hpp"
#include "RequestPipe.hpp"
#include "ResponsePipe.hpp"

AConnection::AConnection() {
  gettimeofday(&lastTimeActive, NULL);
  bodySize = std::string::npos;
}

AConnection::AConnection(AConnection const &other) { *this = other; }

AConnection::~AConnection() {
  while (_writeStreams.empty() == false) {
    delete _writeStreams.front();
    _writeStreams.pop();
  }
}

AConnection &AConnection::operator=(AConnection const &other) {
  if (this != &other) {
    client = other.client;
    host = other.host;
    headSizeLimit = other.headSizeLimit;
    bodySize = other.bodySize;
    headDelimiter = other.headDelimiter;
    _writeBufferSize = other._writeBufferSize;
    _writeStreams = other._writeStreams;
    _readBuffer = other._readBuffer;
    lastTimeActive = other.lastTimeActive;
  }
  return *this;
}

void AConnection::send(std::istream *msg) {
  if (_writeStreams.empty()) Poll::addPollEvent(POLLOUT, this);
  try {
    _writeStreams.push(msg);
  } catch (std::bad_alloc const &) {
    std::cerr << "TEST" << std::endl;
    delete msg;
    throw;
  }
}

void AConnection::onPollEvent(struct pollfd &pollfd) {
  if (!(pollfd.revents & POLLIN || pollfd.revents & POLLOUT)) {
    onNoPollEvent(pollfd);
    return;
  }
  if (pollfd.revents & POLLIN) onPollIn(pollfd);
  if (pollfd.revents & POLLOUT) onPollOut(pollfd);
  gettimeofday(&lastTimeActive, NULL);
}

void AConnection::onPollOut(struct pollfd &pollfd) {
  std::istream *stream;
  ssize_t lenSent;

  if (_writeStreams.empty()) {
    std::cerr << "Error: Write stream is empty" << std::endl;
    return;
  }
  pollfd.revents &= ~POLLOUT;
  stream = _writeStreams.front();
  stream->read(_writeBuffer, BUFFER_SIZE);
  _writeBufferSize = stream->gcount();
  if (stream->eof()) {
    delete _writeStreams.front();
    _writeStreams.pop();
    if (_writeStreams.empty()) pollfd.events &= ~POLLOUT;
  } else if (stream->fail())
    throw std::runtime_error("AConnection::onPollOut(): std::istream.read()");
  lenSent = ::send(pollfd.fd, _writeBuffer, _writeBufferSize, 0);
  if (lenSent == -1) {
    throw std::runtime_error(
        std::string("AConnection::onPollOut(): ::send(): ") +
        std::strerror(errno));
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
  timeout = TIMEOUT - (delta.tv_sec * 1000 + delta.tv_usec / 1000);
  if (timeout <= 0) {
    pollfd.events = 0;
    return;
  }
  Poll::setTimeout(timeout);
}

/**
 * CGI
 */
void AConnection::runCGI(std::string program, std::vector<std::string> &arg,
                         std::vector<std::string> &env) {
  int pid;
  int ResponsePipeFd[2];
  int RequestPipeFd[2];

  if (pipe(ResponsePipeFd) == -1)
    throw std::runtime_error(std::string("AConnection::runCGI(): ") +
                             std::strerror(errno));
  if (pipe(RequestPipeFd) == -1)
    throw std::runtime_error(std::string("AConnection::runCGI(): ") +
                             std::strerror(errno));
  pid = fork();
  if (pid == -1)
    throw std::runtime_error(std::string("AConnection::runCGI(): ") +
                             std::strerror(errno));
  if (pid == 0) {
    Poll::cleanUp();
    if (dup2(ResponsePipeFd[1], STDOUT_FILENO) == -1) {
      std::cerr << "webserv: error: dup2(): " << std::strerror(errno)
                << std::endl;
      exit(EXIT_FAILURE);
    }
    if (dup2(RequestPipeFd[0], STDIN_FILENO) == -1) {
      std::cerr << "webserv: error: dup2(): " << std::strerror(errno)
                << std::endl;
      exit(EXIT_FAILURE);
    }
    close(ResponsePipeFd[0]);
    close(ResponsePipeFd[1]);
    close(RequestPipeFd[0]);
    close(RequestPipeFd[1]);
    std::vector<char *> c_arg;
    std::vector<char *> c_env;
    c_arg.push_back(const_cast<char *>(program.c_str()));
    for (std::vector<std::string>::iterator it = arg.begin(); it != arg.end();
         ++it)
      c_arg.push_back(const_cast<char *>(it->c_str()));
    c_arg.push_back(NULL);
    for (std::vector<std::string>::iterator it = env.begin(); it != env.end();
         ++it)
      c_env.push_back(const_cast<char *>(it->c_str()));
    c_env.push_back(NULL);
    execve(program.c_str(), c_arg.data(), c_env.data());
    std::cerr << "webserv: error: execve(): " << std::strerror(errno)
              << std::endl;
    exit(EXIT_FAILURE);
  }
  close(ResponsePipeFd[1]);
  close(RequestPipeFd[0]);
  struct pollfd pollfd;

  pollfd.fd = ResponsePipeFd[0];
  pollfd.events = POLLIN;
  pollfd.revents = 0;
  Poll::add(new ResponsePipe(this, pid));
  Poll::add(pollfd);

  pollfd.fd = RequestPipeFd[1];
  pollfd.events = POLLOUT;
  pollfd.revents = 0;
  Poll::add(new RequestPipe(this));
  Poll::add(pollfd);
}

void AConnection::cgiSend(std::string msg) { _cgiWriteBuffer += msg; }

void AConnection::onPipePollOut(struct pollfd &pollfd) {
  size_t lenToSend;
  ssize_t lenSent;

  if (pollfd.revents & POLLHUP) {
    pollfd.events = 0;
    return;
  }
  pollfd.revents &= ~POLLOUT;
  if (_cgiWriteBuffer.size() > BUFFER_SIZE)
    lenToSend = BUFFER_SIZE;
  else
    lenToSend = _cgiWriteBuffer.size();
  lenSent = ::write(pollfd.fd, _cgiWriteBuffer.data(), lenToSend);
  if (lenSent == -1)
    std::runtime_error(std::string("AConnection::onPipePollOut(): ") +
                       std::strerror(errno));
  _cgiWriteBuffer.erase(0, lenSent);
  if (_cgiWriteBuffer.empty()) {
    pollfd.events &= ~POLLOUT;
  }
}

void AConnection::closeConnection() { Poll::clearPollEvent(POLLIN, this); }
