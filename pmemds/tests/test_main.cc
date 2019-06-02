
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    //GTEST_FLAG(catch_exceptions);
    return RUN_ALL_TESTS();
}
