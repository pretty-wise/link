/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"

TEST(Gtest, Compile) {
	int a = 3;
	EXPECT_TRUE(a == 3);
}
