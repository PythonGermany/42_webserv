#ifndef RESPONSEPIPE_HPP
#define RESPONSEPIPE_HPP

#include <unistd.h>

#include <string>

#include "CallbackPointer.hpp"
#include "timeval.hpp"

class AConnection;

class ResponsePipe : public IFileDescriptor {
 public:
  ResponsePipe();
  ResponsePipe(AConnection *callbackObject, pid_t cgiPid, int fd);
  ResponsePipe(ResponsePipe const &other);
  ~ResponsePipe();
  ResponsePipe &operator=(ResponsePipe const &other);

  void onPollEvent(struct pollfd &pollfd, CallbackPointer *newCallbackObject,
                   struct pollfd *newPollfd);
  int getFd() const;
  short getFlags() const;

 private:
  AConnection *_callbackObject;
  short _callbackObjectFlags;
  std::string _readBuffer;
  struct timeval lastTimeActive;
  pid_t _cgiPid;
  int _fd;

  void onNoPollEvent(struct pollfd &pollfd);
};

#endif  // RESPONSEPIPE_HPP
