#include "gtest/gtest.h"
#include "../../include/Address.hpp"

TEST(AddressConstructor, validInput)
{
    EXPECT_NO_THROW(Address("127.0.0.1", "8080"));
    EXPECT_NO_THROW(Address("192.241.43.0", "100"));
    EXPECT_NO_THROW(Address("::", "123"));
    EXPECT_NO_THROW(Address("[::1]", "4321"));
}

TEST(AddressConstructor, invalidInput)
{
    EXPECT_THROW(Address("google.com", "8080"), std::invalid_argument);
    EXPECT_THROW(Address("0.0.0.0", ""), std::invalid_argument);
    EXPECT_THROW(Address("::", "-12"), std::invalid_argument);
    EXPECT_THROW(Address("1234.1.1.1", "12"), std::invalid_argument);
    EXPECT_THROW(Address("[::]", "1212121212121"), std::invalid_argument);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}