/*
 * bitopsTests.cpp
 *
 *  Created on: 10 ago 2017
 *      Author: elcab_000
 */

#include "gtest/gtest.h"
#include "./../bitops.h"
namespace {

	TEST(bitopsTest, bitCnt)
	{
		ASSERT_EQ(bitCnt(0), 0u);
		ASSERT_EQ(bitCnt(3), 2u);
		ASSERT_EQ(bitCnt(0x8000000000000000), 1u);
		ASSERT_EQ(bitCnt(2305856495478640774ull), 11u);
	}

	TEST(bitopsTest, firstOne)
	{
		//ASSERT_EQ(firstOne(0), A1);
		ASSERT_EQ(firstOne(3), A1);
		ASSERT_EQ(firstOne(0x8000000000000000), H8);
		ASSERT_EQ(firstOne(2305856495478640774ull), B1);
	}

	TEST(bitopsTest, iterateBit)
	{
		bitMap b = 3;

		ASSERT_EQ(iterateBit(b), A1);
		ASSERT_EQ(b, 2ull);

		b = 0x8000000000000000;

		ASSERT_EQ(iterateBit(b), H8);
		ASSERT_EQ(b, 0ull);

		b = 2305856495478640774ull;

		ASSERT_EQ(iterateBit(b), B1);
		ASSERT_EQ(b, 2305856495478640772ull);

	}

	TEST(bitopsTest, isSquareInBitmap)
	{
		bitMap b = 3;

		ASSERT_TRUE(isSquareInBitmap(b, A1));
		ASSERT_TRUE(isSquareInBitmap(b, B1));
		ASSERT_FALSE(isSquareInBitmap(b, D7));


	}

}


