/*
 * AllTest.cpp
 *
 *  Created on: 10 ago 2017
 *      Author: elcab_000
 */

#include "gtest/gtest.h"
//#include "FooTest.cpp"
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


