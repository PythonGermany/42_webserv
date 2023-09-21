#include "gtest/gtest.h"
#include "timeval.hpp"

TEST(Timeval, Subtraction)
{
    struct timeval a = {.tv_sec = 42, .tv_usec = 42};
    struct timeval b = {.tv_sec = 42, .tv_usec = 42};

    struct timeval c = a - b;
    EXPECT_EQ(c.tv_sec, 0);
    EXPECT_EQ(c.tv_usec, 0);

    a = (struct timeval){.tv_sec = 42, .tv_usec = 21};
    b = (struct timeval){.tv_sec = 21, .tv_usec = 42};

    c = a - b;
    EXPECT_EQ(c.tv_sec, 20);
    EXPECT_EQ(c.tv_usec, 1000000 - 21);
}