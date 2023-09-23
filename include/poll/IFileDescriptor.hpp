#ifndef IFILEDESCRIPTOR_HPP
#define IFILEDESCRIPTOR_HPP

#include <poll.h>

#define POLLINACTIVE POLLERR

class IFileDescriptor {
 public:
  virtual ~IFileDescriptor(){};
  virtual void onPollEvent(struct pollfd &pollfd) = 0;
};

#endif  // IFILEDESCRIPTOR_HPP
