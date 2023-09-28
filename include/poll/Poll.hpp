#ifndef POLL_HPP
#define POLL_HPP

#include <sys/time.h>

#include <csignal>
#include <vector>

#include "AConnection.hpp"
#include "IFileDescriptor.hpp"
#include "webserv.hpp"

class Poll {
 public:
  typedef std::vector<IFileDescriptor *>::size_type size_type;

  static void add(IFileDescriptor *src, struct pollfd const &pollfd);
  static void remove(size_type pos);
  static void remove(IFileDescriptor *src);
  static bool poll();
  static void signalHandler(int);
  static void setTimeout(int src);
  static void setPollActive(short oldEvents, IFileDescriptor *src);
  static void addPollEvent(short event, IFileDescriptor *src);
  static void clearPollEvent(short event, IFileDescriptor *src);
  static short setPollInactive(IFileDescriptor *src);
  static void cleanUp();
  static pid_t lastForkPid();
  static void lastForkPid(pid_t src);

 private:
  bool stop;
  int timeout;
  pid_t pid;
  size_t pos;
  struct sigaction originalSigAction;
  std::vector<IFileDescriptor *> callbackObjects;
  std::vector<struct pollfd> pollfds;

  Poll();
  Poll(Poll const &other);
  ~Poll();
  Poll &operator=(Poll const &other);

  static Poll &getInstance();
  void iterate();
};

#endif  // POLL_HPP
