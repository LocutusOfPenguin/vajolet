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





/*	\brief tell wheter a square is present in a bitmap
	\author Marco Belli
	\version 1.0
	\date 13/05/2017
*/
static inline bool isSquareInBitmap(const bitMap b, const tSquare sq)
{
	return b & bitHelper::getBitmapFromSquare(sq);
}
/*	\brief tell wheter a square defined by file and rank is present in a bitmap
	\author Marco Belli
	\version 1.0
	\date 13/05/2017
*/
static inline bool isSquareInBitmap(const bitMap b, const unsigned int file, const unsigned int rank)
{
	return b & bitHelper::getBitmapFromSquare(file,rank);
}



/*	\brief return true if the bitmap has more than one bit set
	\author Marco Belli
	\version 1.1
	\date 17/06/2017
*/
inline bool moreThanOneBit(const bitMap b)
{
	return bitCnt(b)>1;
	//return b & (b - 1); // old code, now a simply bitcount seems faster
}



//-----------------------------------------------------------------------------
//	function prototype
//-----------------------------------------------------------------------------
std::string displayBitmap(const bitMap b);


#endif /* BITOPS_H_ */
