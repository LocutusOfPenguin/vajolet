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

#ifndef DATA_H_
#define DATA_H_
#include "vajolet.h"

//------------------------------------------------
//enum
//------------------------------------------------



//------------------------------------------------
//	const
//------------------------------------------------

//------------------------------------------------
//	extern variables
//------------------------------------------------

class bitHelper
{
private:
	static bitMap BITSET[squareNumber+1];
	static tSquare BOARDINDEX[8][8];
	static  const int FILES[squareNumber];
	static  const int RANKS[squareNumber];
	static const int SQUARE_COLOR[squareNumber];
	static bitMap centerBitmap;
	static bitMap bigCenterBitmap;
public:
	static void initbitHelper(void);
	/*	\brief set the Nth bit to 1
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	static inline bitMap getBitmapFromSquare(const tSquare n)
	{
		assert( n < squareNumber+1 );
		return BITSET[n];
	}

	/*	\brief get a tSquare from file and rank
		\author Marco Belli
		\version 1.0
		\date 13/05/2017
	*/
	static inline tSquare getTsquareFromFileRank(const unsigned int file, const unsigned int rank)
	{
		assert( file < 8 );
		assert( rank < 8 );
		return BOARDINDEX[file][rank];
	}

	static inline bitMap getBitmapFromSquare(const unsigned int file, const unsigned int rank)
	{
		assert(file < 8);
		assert(rank < 8);
		return getBitmapFromSquare( getTsquareFromFileRank( file, rank ) );
	}

	static inline int getFile(const tSquare n)
	{
		assert( n < squareNumber );
		return FILES[n];
	}
	static inline int getRank(const tSquare n)
	{
		assert( n < squareNumber );
		return RANKS[n];
	}
	static inline int getSquareColor(const tSquare n)
	{
		assert( n < squareNumber );
		return SQUARE_COLOR[n];
	}
	static inline bitMap getCenterBitmap(void)
	{
		return centerBitmap;
	}
	static inline bitMap getExtCenterBitmap(void)
	{
		return bigCenterBitmap;
	}

	static inline bitMap getBigCenterBitmap(void)
	{
		return centerBitmap | bigCenterBitmap;
	}
};


extern bitMap RANKMASK[squareNumber];
extern bitMap FILEMASK[squareNumber];
extern bitMap DIAGA1H8MASK[squareNumber];
extern bitMap DIAGA8H1MASK[squareNumber];
extern bitMap SQUARES_BETWEEN[squareNumber][squareNumber];
extern bitMap LINES[squareNumber][squareNumber];
extern bitMap ISOLATED_PAWN[squareNumber];
extern bitMap PASSED_PAWN[2][squareNumber];
extern bitMap SQUARES_IN_FRONT_OF[2][squareNumber];

extern bitMap BITMAP_COLOR[2];
extern int SQUARE_DISTANCE[squareNumber][squareNumber];




//------------------------------------------------
//	inline functions
//------------------------------------------------


inline bitMap operator += (bitMap& b, const tSquare sq) { b = b | bitHelper::getBitmapFromSquare(sq); return b; }


//------------------------------------------------
//	function prototype
//------------------------------------------------
void initData(void);

#endif /* DATA_H_ */
