#ifndef TIMEVAL_HPP
#define TIMEVAL_HPP

#include <sys/time.h>

timeval operator-(timeval const &lhs, timeval const &rhs);

#endif  // TIMEVAL_HPP
