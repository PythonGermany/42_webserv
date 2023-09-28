#include "Poll.hpp"

#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "webserv.hpp"

/**
 * @brief Add callback object with corresponding pollfd struct
 * @param pollfd Poll takes ownership of the fd element and ensures its
 * proper closure.
 * @param src Poll takes ownersip of the allocated callback object and ensures
 * proper cleanup
 */
void Poll::add(CallbackPointer const &src, struct pollfd const &pollfd) try {
  getInstance().callbackObjects.push_back(src);
  try {
    getInstance().pollfds.push_back(pollfd);
  } catch (...) {
    getInstance().callbackObjects.pop_back();
    throw;
  }
} catch (...) {
  if (src.link == false) delete src.ptr;
  close(pollfd.fd);
  throw;
}

// void Poll::remove(size_type pos) {
//   Poll &poll = getInstance();

//   delete poll.callbackObjects[pos];
//   close(poll.pollfds[pos].fd);
//   poll.callbackObjects.erase(poll.callbackObjects.begin() + pos);
//   poll.pollfds.erase(poll.pollfds.begin() + pos);
//   if (pos <= poll.pos) --poll.pos;
//   std::cout << "Poll delete: " << pos << std::endl;
// }

void Poll::remove(IFileDescriptor *src) {
  for (size_t i = 0; i < callbackObjects.size(); ++i) {
    if (callbackObjects[i].ptr == src) {
      if (callbackObjects[i].link == false) delete callbackObjects[i].ptr;
      close(pollfds[i].fd);
      callbackObjects.erase(callbackObjects.begin() + i);
      pollfds.erase(pollfds.begin() + i);
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

  std::vector<CallbackPointer>::iterator callbackObjectsIt =
      poll.callbackObjects.begin();
  std::vector<struct pollfd>::iterator pollfdIt = poll.pollfds.begin();
  while (callbackObjectsIt != poll.callbackObjects.end()) {
    if (callbackObjectsIt->link == false) delete callbackObjectsIt->ptr;
    close(pollfdIt->fd);
    ++callbackObjectsIt;
    ++pollfdIt;
  }
  poll.callbackObjects.clear();
  poll.pollfds.clear();
  sigaction(SIGINT, &poll.originalSigAction, NULL);
}

Poll::~Poll() { cleanUp(); }

void Poll::iterate() {
  for (size_t i = 0; i < callbackObjects.size();) {
    struct pollfd newPollfd[2];
    CallbackPointer newCallbackObject[2];
    bool elementRemoved;

    for (size_t j = 0; j < sizeof(newPollfd) / sizeof(*newPollfd); ++j)
      newPollfd[j].fd = -1;
    elementRemoved = false;
    try {
      if (callbackObjects[i].link)
        std::cerr << "try to call link: " << pollfds[i].fd << std::endl;
      else
        std::cerr << "try to call no link" << pollfds[i].fd << std::endl;
      callbackObjects[i].ptr->onPollEvent(pollfds[i], newCallbackObject,
                                          newPollfd);

      for (size_t j = 0;
           j < sizeof(newCallbackObject) / sizeof(*newCallbackObject); ++j)
        if (newCallbackObject[j].ptr != NULL) {
          callbackObjects.push_back(newCallbackObject[j]);
          try {
            pollfds.push_back(newPollfd[j]);
          } catch (...) {
            callbackObjects.pop_back();
            throw;
          }
          if (newCallbackObject[j].link)
            std::cerr << "link created" << std::endl;
        }
      if (pollfds[i].events == 0) {
        close(pollfds[i].fd);
        if (callbackObjects[i].link == false) delete callbackObjects[i].ptr;
        callbackObjects.erase(callbackObjects.begin() + i);
        pollfds.erase(pollfds.begin() + i);
        elementRemoved = true;
      }
    } catch (std::exception const &e) {
      std::cerr << e.what() << '\n';
      remove(callbackObjects[i].ptr);
      i = 0;
      elementRemoved = true;
      for (size_t j = 0;
           j < sizeof(newCallbackObject) / sizeof(*newCallbackObject); ++j) {
        if (newCallbackObject[j].ptr != NULL &&
            newCallbackObject[j].link == false)
          delete newCallbackObject[j].ptr;
        if (newPollfd[j].fd != -1) close(newPollfd[j].fd);
      }
    }
    if (elementRemoved == false) ++i;
  }
}

void Poll::setTimeout(int src) {
  Poll &poll = getInstance();

  if (poll.timeout == -1 || src < poll.timeout) poll.timeout = src;
}

void Poll::addPollEvent(short event, IFileDescriptor *src) {
  std::vector<CallbackPointer>::iterator callbackObjectsIt =
      getInstance().callbackObjects.begin();
  std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
  while (callbackObjectsIt != getInstance().callbackObjects.end()) {
    if (callbackObjectsIt->link == false && callbackObjectsIt->ptr == src) {
      pollfdIt->events |= event;
      return;
    }
    ++callbackObjectsIt;
    ++pollfdIt;
  }
  throw std::invalid_argument("Poll::addPollEvent");
}

void Poll::clearPollEvent(short event, IFileDescriptor *src) {
  std::vector<CallbackPointer>::iterator callbackObjectsIt =
      getInstance().callbackObjects.begin();
  std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
  while (callbackObjectsIt != getInstance().callbackObjects.end()) {
    if (callbackObjectsIt->link == false && callbackObjectsIt->ptr == src) {
      pollfdIt->events &= ~event;
      return;
    }
    ++callbackObjectsIt;
    ++pollfdIt;
  }
  throw std::invalid_argument("Poll::addPollEvent");
}

void Poll::setPollActive(short oldEvents, IFileDescriptor *src) {
  std::vector<CallbackPointer>::iterator callbackObjectsIt =
      getInstance().callbackObjects.begin();
  std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
  while (callbackObjectsIt != getInstance().callbackObjects.end()) {
    if (callbackObjectsIt->link == false && callbackObjectsIt->ptr == src) {
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
  std::vector<CallbackPointer>::iterator callbackObjectsIt =
      getInstance().callbackObjects.begin();
  std::vector<struct pollfd>::iterator pollfdIt = getInstance().pollfds.begin();
  while (callbackObjectsIt != getInstance().callbackObjects.end()) {
    if (callbackObjectsIt->link == false && callbackObjectsIt->ptr == src) {
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
