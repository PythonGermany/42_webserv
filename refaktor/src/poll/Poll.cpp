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
 * @throw std::vector::push_back() might throw std::bad_alloc()
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

/**
 * @brief removed callback object and pollfd struct by index
 * @throw noexcept
 */
void Poll::remove(size_t pos) {
  if (callbackObjects[pos].link == false) delete callbackObjects[pos].ptr;
  close(pollfds[pos].fd);
  callbackObjects.erase(callbackObjects.begin() + pos);
  pollfds.erase(pollfds.begin() + pos);
}

/**
 * @param src all occurences with this value will be removed
 * @throw noexcept
 */
void Poll::remove(IFileDescriptor *src) {
  std::cerr << "remove: " << src << std::endl;
  for (size_t i = 0; i < callbackObjects.size();) {
    if (callbackObjects[i].ptr == src) {
      remove(i);
    } else
      ++i;
  }
}

void Poll::release(CallbackPointer const *callback, struct pollfd const *pollfd,
                   size_t size) {
  for (size_t i = 0; i < size; ++i) {
    if (callback[i].ptr != NULL && callback[i].link == false)
      delete callback[i].ptr;
    if (pollfd[i].fd != -1) close(pollfd[i].fd);
  }
}

bool Poll::poll() {
  Poll &poll = getInstance();

  std::cout << "poll.timeout: " << poll.timeout << std::endl;
  int ready = ::poll(poll.pollfds.data(), poll.pollfds.size(), poll.timeout);
  if (poll.stop) return false;
  if (ready == -1)
    throw std::runtime_error(std::string("Poll::poll(): ") +
                             std::strerror(errno));

  if (ready == 0) std::cerr << "Poll: no revents" << std::endl;
  poll.timeout = -1;
  poll.iterate();
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

/**
 * @brief is used to release resources in child processes
 * @throw noexcept
 */
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

/**
 * @brief
 * Tries to add array.
 * Only guarantees that either both callback object and pollfd struct are added
 * or neither. Does not close fds or delete allocated memory.
 * @throw std::vector::push_back() might throw std::bad_alloc()
 */
void Poll::tryToAddNewElements(CallbackPointer const *callback,
                               struct pollfd const *pollfd, size_t size) {
  for (size_t i = 0; i < size; ++i)
    if (callback[i].ptr != NULL) {
      callbackObjects.push_back(callback[i]);
      try {
        pollfds.push_back(pollfd[i]);
      } catch (...) {
        callbackObjects.pop_back();
        throw;
      }
      if (callback[i].link) std::cerr << "link created" << std::endl;
    }
}

void Poll::iterate() {
  for (size_t i = 0; i < callbackObjects.size();) {
    struct pollfd newPollfd[2];
    CallbackPointer newCallbackObject[2];

    for (size_t j = 0; j < sizeof(newPollfd) / sizeof(*newPollfd); ++j)
      newPollfd[j].fd = -1;
    try {
      callbackObjects[i].ptr->onPollEvent(pollfds[i], newCallbackObject,
                                          newPollfd);
      tryToAddNewElements(newCallbackObject, newPollfd,
                          sizeof(newPollfd) / sizeof(*newPollfd));

      if (pollfds[i].events == 0) {
        remove(i);
      } else {
        ++i;
      }
    } catch (std::exception const &e) {
      std::cerr << e.what() << '\n';
      remove(callbackObjects[i].ptr);
      release(newCallbackObject, newPollfd,
              sizeof(newPollfd) / sizeof(*newPollfd));
      i = 0;
    }
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
