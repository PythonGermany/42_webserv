#ifndef ACONNECTION_HPP
#define ACONNECTION_HPP

#include <unistd.h>

#include <list>
#include <set>
#include <string>

#define BUFFER_SIZE 65536

class AConnection {
 protected:
  static std::list<AConnection *> _queue;

  int _fd;
  bool _listenIn;
  bool _listenOut;

 private:
  short _state;

 public:
  AConnection();
  AConnection(int fd, bool listenIn, bool listenOut);
  virtual ~AConnection();

  typedef enum state_e { ERROR = 1, TIMEOUT = 2 } state_t;

  static int pushQueueFront(AConnection *connection);
  static AConnection *popQueueFront();

  int fd();

  virtual void in() = 0;
  virtual void out() = 0;
  virtual void process() = 0;

  bool listenIn();
  bool listenOut();

  void setStateBits(state_t bits);
  void clearStateBits(state_t bits);
  bool getStateBit(state_t bit);

  bool remove();
};

#endif
