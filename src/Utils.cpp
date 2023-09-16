#include "../include/Utils.hpp"

timeval operator-(timeval const &lhs, timeval const &rhs)
{
	timeval result;
    result.tv_sec = lhs.tv_sec - rhs.tv_sec;
    result.tv_usec = lhs.tv_usec - rhs.tv_usec;

    if (result.tv_usec < 0) {
        result.tv_sec -= 1;
        result.tv_usec += 1000000;
    }
    return result;
}
