#ifndef IFILEDESCRIPTOR_HPP
#define IFILEDESCRIPTOR_HPP

struct pollfd;
class CallbackPointer;

#define POLLINACTIVE POLLERR

class IFileDescriptor {
 public:
  virtual ~IFileDescriptor();
  virtual void onPollEvent(struct pollfd &pollfd,
                           CallbackPointer *newCallbackObject,
                           struct pollfd *newPollfd) = 0;
};

#endif  // IFILEDESCRIPTOR_HPP
