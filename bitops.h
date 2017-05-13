/*
	This file is part of Vajolet.

    Vajolet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vajolet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vajolet.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef BITOPS_H_
#define BITOPS_H_

#include <string>
#include "vajolet.h"
#include "data.h"


//-----------------------------------------------------------------------------
//	inline function
//-----------------------------------------------------------------------------

/*	\brief count the number of 1 bits in a 64 bit variable
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static inline unsigned int bitCnt(const bitMap bitmap)
{
	return __builtin_popcountll(bitmap);
}


/*	\brief get the index of the rightmost one bit
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
static inline tSquare firstOne(const bitMap bitmap)
{
	return (tSquare)__builtin_ctzll(bitmap);
}

/*	\brief get the index of the rightmost one bit and clear the bit in the bitmap
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
static inline tSquare iterateBit(bitMap & b)
{
	const tSquare t = firstOne(b);
	b &= ( b - 1 );
	return t;

}



/*	\brief get a tSquare from file and rank
	\author Marco Belli
	\version 1.0
	\date 13/05/2017
*/
static inline tSquare getTsquareFromFileRank(const unsigned int file, const unsigned int rank)
{
	return BOARDINDEX[file][rank];
}

/*	\brief set the Nth bit to 1
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline bitMap getBitmapFromSquare(const tSquare n)
{
	return BITSET[n];
}

inline bitMap getBitmapFromSquare(const unsigned int file, const unsigned int rank)
{
	return getBitmapFromSquare(getTsquareFromFileRank(file,rank));
}

/*	\brief tell wheter a square is present in a bitmap
	\author Marco Belli
	\version 1.0
	\date 13/05/2017
*/
static inline bool isSquareInBitmap(const bitMap b, const tSquare sq)
{
	return b & getBitmapFromSquare(sq);
}
/*	\brief tell wheter a square defined by file and rank is present in a bitmap
	\author Marco Belli
	\version 1.0
	\date 13/05/2017
*/
static inline bool isSquareInBitmap(const bitMap b, const unsigned int file, const unsigned int rank)
{
	return b & getBitmapFromSquare(file,rank);
}



/*	\brief return true if the bitmap has more than one bit set
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline bool moreThanOneBit(const bitMap b)
{
  return b & (b - 1);
}

/*	\brief return true if the 3 squares are aligned
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline bool squaresAligned(tSquare s1, tSquare s2, tSquare s3)
{
	return LINES[s1][s2] & getBitmapFromSquare(s3);
	/*return  (SQUARES_BETWEEN[s1][s2] | SQUARES_BETWEEN[s1][s3] | SQUARES_BETWEEN[s2][s3])
			& (     bitSet(s1) |        bitSet(s2) |        bitSet(s3));*/
}

//-----------------------------------------------------------------------------
//	function prototype
//-----------------------------------------------------------------------------
std::string displayBitmap(const bitMap b);


#endif /* BITOPS_H_ */
