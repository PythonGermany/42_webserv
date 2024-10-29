#ifndef POLL_HPP
#define POLL_HPP

#include <csignal>
#include <vector>

class CallbackPointer;
class IFileDescriptor;

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
  struct sigaction originalSigIntAction;
  struct sigaction originalSigTermAction;
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
};

#endif  // POLL_HPP
