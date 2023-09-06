#include "gtest/gtest.h"
#include "../../include/Address.hpp"

TEST(AddressConstructor, validInput)
{
    EXPECT_NO_THROW(Address("127.0.0.1", "8080"));
    EXPECT_NO_THROW(Address("192.241.43.0", "100"));
    EXPECT_NO_THROW(Address("::", "123"));
    EXPECT_NO_THROW(Address("[::1]", "4321"));
    EXPECT_NO_THROW(Address());
    EXPECT_NO_THROW(Address(Address("0.0.0.0", "0")));
}

TEST(AddressConstructor, invalidInput)
{
    EXPECT_THROW(Address("google.com", "8080"), std::invalid_argument);
    EXPECT_THROW(Address("0.0.0.0", ""), std::invalid_argument);
    EXPECT_THROW(Address("::", "-12"), std::invalid_argument);
    EXPECT_THROW(Address("1234.1.1.1", "12"), std::invalid_argument);
    EXPECT_THROW(Address("[::]", "1212121212121"), std::invalid_argument);
}

TEST(AddressMethod, family)
{
    Address a("127.0.0.1", "80");
    EXPECT_EQ(a.family(), AF_INET);
    Address b("[::]", "42");
    EXPECT_EQ(b.family(), AF_INET6);
    Address c;
    EXPECT_EQ(c.family(), 0);
}

TEST(AddressMethod, port)
{
    Address a("127.0.0.1", "80");
    EXPECT_EQ(a.port(), 80);
    Address b("[::]", "42");
    EXPECT_EQ(b.port(), 42);
}

TEST(AddressMethod, data)
{
    Address a("127.0.0.1", "80");
    sockaddr_in *ptoa;

    ptoa = (sockaddr_in *)a.data();
    EXPECT_EQ(a.family(), ptoa->sin_family);

    Address b("::1", "423");
    sockaddr_in6 *ptob;
    
    ptob = (sockaddr_in6 *)b.data();
    EXPECT_EQ(b.family(), ptob->sin6_family);
}

TEST(AddressMethod, size)
{
    Address a("0.1.2.3", "1234");

    EXPECT_EQ(a.size(), sizeof(sockaddr_in));

    Address b("1:2:3:4::6:7:8", "1234");
    EXPECT_EQ(b.size(), sizeof(sockaddr_in6));

    Address c;
    EXPECT_NO_THROW(c.size(sizeof(sockaddr_in)));
    EXPECT_EQ(c.size(), sizeof(sockaddr_in));
    EXPECT_EQ(c.family(), AF_INET);

    Address d;
    EXPECT_NO_THROW(d.size(sizeof(sockaddr_in6)));
    EXPECT_EQ(d.size(), sizeof(sockaddr_in6));
    EXPECT_EQ(d.family(), AF_INET6);

    EXPECT_THROW(Address().size(0), std::invalid_argument);
}

/**
 * tests with Constructor, Copy Constructor and assignment operator
*/
TEST(AddressOperator, StreamInsertion)
{
    std::stringstream ss;
    ss << Address(Address("127.0.0.1", "80"));
    EXPECT_EQ(ss.str(), "127.0.0.1:80");
    ss.str("");
    
    Address a = Address("[::1]", "8080");
    ss << a;
    EXPECT_EQ(ss.str(), "[::1]:8080");
}
