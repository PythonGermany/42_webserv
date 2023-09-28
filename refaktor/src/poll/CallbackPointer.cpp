#include "CallbackPointer.hpp"

#include <cstddef>  //NULL

CallbackPointer::CallbackPointer() : ptr(NULL), link(false) {}

CallbackPointer::CallbackPointer(CallbackPointer const &other)
    : ptr(other.ptr), link(other.link) {}

CallbackPointer::~CallbackPointer() {}

CallbackPointer &CallbackPointer::operator=(CallbackPointer const &other) {
  ptr = other.ptr;
  link = other.link;
  return *this;
}
