#ifndef POLL_HPP
#define POLL_HPP

#include <sys/time.h>

#include <csignal>
#include <vector>

#include "CallbackPointer.hpp"
#include "global.hpp"

// WEBSERV_CONFIG ----------- POLL VALUES ---------------------------
/**
 * close connections if they are TIMEOUT milliseconds inactive
 */
#define CONNECTION_TIMEOUT 30000
#define CGI_TIMEOUT 3000

class Poll {
 public:
  static void add(CallbackPointer const &src, struct pollfd const &pollfd);
  void tryToAddNewElements(CallbackPointer const *callback,
                           struct pollfd const *pollfd, size_t size);
  static bool poll();
  static void signalHandler(int);
  static void setTimeout(int src);
  static void setPollActive(short oldEvents, IFileDescriptor *src);
  static void addPollEvent(short event, IFileDescriptor *src);
  static void addPollEvent(short event, int fd);
  static void clearPollEvent(short event, IFileDescriptor *src);
  static void clearPollEvent(short event, int fd);
  static short setPollInactive(IFileDescriptor *src);
  static void cleanUp();
  static pid_t lastForkPid();
  static void lastForkPid(pid_t src);

 private:
  bool stop;
  int timeout;
  pid_t pid;
  // size_t pos;
  struct sigaction originalSigAction;
  std::vector<CallbackPointer> callbackObjects;
  std::vector<struct pollfd> pollfds;

  Poll();
  Poll(Poll const &other);
  ~Poll();
  Poll &operator=(Poll const &other);

  static Poll &getInstance();
  void iterate();
  void remove(IFileDescriptor *src);
  void remove(size_t pos);
  void release(CallbackPointer const *callback, struct pollfd const *pollfd,
               size_t size);
  void handleAddingAttempts();
};

#endif  // POLL_HPP
