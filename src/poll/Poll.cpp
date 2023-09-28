#include "Poll.hpp"

#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "Log.hpp"

/**
 * @brief Add callback object with corresponding pollfd struct
 * @param pollfd Poll takes ownership of the fd element and ensures its
 * proper closure.
 * @param src Poll takes ownersip of the allocated callback object and ensures
 * proper cleanup
 */
void Poll::add(IFileDescriptor *src, struct pollfd const &pollfd) try {
  getInstance().callbackObjects.push_back(src);
  try {
    getInstance().pollfds.push_back(pollfd);
  } catch (...) {
    getInstance().callbackObjects.pop_back();
    throw;
  }
} catch (...) {
  delete src;
  close(pollfd.fd);
}

void Poll::remove(size_type pos) {
  Poll &poll = getInstance();

  delete poll.callbackObjects[pos];
  close(poll.pollfds[pos].fd);
  poll.callbackObjects.erase(poll.callbackObjects.begin() + pos);
  poll.pollfds.erase(poll.pollfds.begin() + pos);
  if (pos <= poll.pos) --poll.pos;
  std::cout << "Poll delete: " << pos << std::endl;
}

void Poll::remove(IFileDescriptor *src) {
  Poll &poll = getInstance();
  size_t size = poll.callbackObjects.size();

  for (size_t i = 0; i < size; ++i) {
    if (poll.callbackObjects[i] == src) {
      delete src;
      close(poll.pollfds[i].fd);
      poll.callbackObjects.erase(poll.callbackObjects.begin() + i);
      poll.pollfds.erase(poll.pollfds.begin() + i);
      if (i <= poll.pos) --poll.pos;
      std::cout << "Poll delete: " << i << std::endl;
      return;
    }
  }
}

bool Poll::poll() {
  Poll &poll = getInstance();
  static size_t numListenSockets = poll.callbackObjects.size();

  // std::cout << "poll.timeout: " << poll.timeout << std::endl;
  int ready = ::poll(poll.pollfds.data(), poll.pollfds.size(), poll.timeout);
  if (poll.stop) return false;
  if (ready == -1)
    throw std::runtime_error(std::string("Poll::poll(): ") +
                             std::strerror(errno));
  poll.timeout = -1;
  poll.iterate();
  if (poll.timeout == -1 && numListenSockets != poll.callbackObjects.size())
    poll.timeout = CONNECTION_TIMEOUT;
  return true;
}

/**
 * print newline because ^C is printed without
 */
void Poll::signalHandler(int) {
  getInstance().stop = true;
  std::cout << std::endl;
}

Poll::Poll() {
  struct sigaction sigAction = {};

  sigAction.sa_handler = Poll::signalHandler;
  sigaction(SIGINT, &sigAction, &originalSigAction);
  timeout = -1;
  stop = false;
  pid = -1;
}

pid_t Poll::lastForkPid() { return getInstance().pid; }

void Poll::lastForkPid(pid_t src) { getInstance().pid = src; }

void Poll::cleanUp() {
  Poll &poll = getInstance();

  std::vector<IFileDescriptor *>::iterator callbackObjectsIt =
      poll.callbackObjects.begin();
  std::vector<struct pollfd>::iterator pollfdIt = poll.pollfds.begin();
  while (callbackObjectsIt != poll.callbackObjects.end()) {
    delete *callbackObjectsIt;
    close(pollfdIt->fd);
    ++callbackObjectsIt;
    ++pollfdIt;
  }
  poll.callbackObjects.clear();
  poll.pollfds.clear();
  sigaction(SIGINT, &poll.originalSigAction, NULL);
}

Poll::~Poll() { cleanUp(); }

Poll &Poll::operator=(Poll const &) { return *this; }

/**
 * TODO: find better solution for pos = 0 and remove
 */
void Poll::iterate() {
  pos = 1;
  while (pos <= callbackObjects.size()) {
    try {
      std::cout << "Poll call: " << pos - 1 << " with size: " << pollfds.size()
                << std::endl;
      callbackObjects[pos - 1]->onPollEvent(pollfds[pos - 1]);
    } catch (const std::exception &e) {
      Log::write(e.what() + std::string(" while iterating"), WARNING,
                 BRIGHT_YELLOW);
      pollfds[pos - 1].events = 0;
    }
    if (pollfds[pos - 1].events == 0) {
      remove(pos - 1);
    }
    ++pos;
  }
}

void Poll::setTimeout(int src) {
  Poll &poll = getInstance();

  if (poll.timeout == -1 || src < poll.timeout) poll.timeout = src;
}

void Poll::addPollEvent(short event, IFileDescriptor *src) {
  std::vector<IFileDescriptor *>::iterator callbackObjectsIt =
      getInstance().callbackObjects.begin();
  std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
  while (callbackObjectsIt != getInstance().callbackObjects.end()) {
    if (*callbackObjectsIt == src) {
      pollfdIt->events |= event;
      return;
    }
    ++callbackObjectsIt;
    ++pollfdIt;
  }
  throw std::invalid_argument("Poll::addPollEvent");
}

void Poll::clearPollEvent(short event, IFileDescriptor *src) {
  std::vector<IFileDescriptor *>::iterator callbackObjectsIt =
      getInstance().callbackObjects.begin();
  std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
  while (callbackObjectsIt != getInstance().callbackObjects.end()) {
    if (*callbackObjectsIt == src) {
      pollfdIt->events &= ~event;
      return;
    }
    ++callbackObjectsIt;
    ++pollfdIt;
  }
  throw std::invalid_argument("Poll::addPollEvent");
}

void Poll::setPollActive(short oldEvents, IFileDescriptor *src) {
  std::vector<IFileDescriptor *>::iterator callbackObjectsIt =
      getInstance().callbackObjects.begin();
  std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
  while (callbackObjectsIt != getInstance().callbackObjects.end()) {
    if (*callbackObjectsIt == src) {
      pollfdIt->events &= ~POLLINACTIVE;
      pollfdIt->events |= oldEvents;
      return;
    }
    ++callbackObjectsIt;
    ++pollfdIt;
  }
  throw std::invalid_argument("Poll::addPollEvent");
}

short Poll::setPollInactive(IFileDescriptor *src) {
  std::vector<IFileDescriptor *>::iterator callbackObjectsIt =
      getInstance().callbackObjects.begin();
  std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
  while (callbackObjectsIt != getInstance().callbackObjects.end()) {
    if (*callbackObjectsIt == src) {
      short tmp = pollfdIt->events;
      pollfdIt->events &= ~POLLIN;
      pollfdIt->events |= POLLINACTIVE;
      return tmp;
    }
    ++callbackObjectsIt;
    ++pollfdIt;
  }
  throw std::invalid_argument("Poll::getPollEvent");
}

/**
 * only if an object exists does the destructor get called
 */
Poll &Poll::getInstance() {
  static Poll poll;

  return poll;
}
