#ifndef CALLBACKPOINTER_HPP
#define CALLBACKPOINTER_HPP

class IFileDescriptor;

class CallbackPointer {
 public:
  IFileDescriptor *ptr;
  bool link;

  CallbackPointer();
  CallbackPointer(CallbackPointer const &other);
  ~CallbackPointer();
  CallbackPointer &operator=(CallbackPointer const &other);
};
#endif  // CALLBACKPOINTER_HPP
