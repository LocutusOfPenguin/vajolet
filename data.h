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
	static const Color SQUARE_COLOR[squareNumber];
	static bitMap centerBitmap;
	static bitMap bigCenterBitmap;
	static bitMap BITMAP_COLOR[2];
	static bitMap RANKMASK[squareNumber];
	static bitMap FILEMASK[squareNumber];
	static bitMap SQUARES_BETWEEN[squareNumber][squareNumber];
	static bitMap LINES[squareNumber][squareNumber];
	static unsigned int SQUARE_DISTANCE[squareNumber][squareNumber];
	static bitMap ISOLATED_PAWN[squareNumber];
	static bitMap PASSED_PAWN[2][squareNumber];
	static bitMap SQUARES_IN_FRONT_OF[2][squareNumber];
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
		return (BITSET[n]);
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
		return (BOARDINDEX[file][rank]);
	}

	static inline bitMap getBitmapFromSquare(const unsigned int file, const unsigned int rank)
	{
		assert(file < 8);
		assert(rank < 8);
		return (getBitmapFromSquare( getTsquareFromFileRank( file, rank ) ));
	}

	static inline int getFile(const tSquare n)
	{
		assert( n < squareNumber );
		return (FILES[n]);
	}
	static inline int getRank(const tSquare n)
	{
		assert( n < squareNumber );
		return (RANKS[n]);
	}
	static inline Color getSquareColor(const tSquare n)
	{
		assert( n < squareNumber );
		return (SQUARE_COLOR[n]);
	}
	static inline bitMap getCenterBitmap(void)
	{
		return (centerBitmap);
	}
	static inline bitMap getExtCenterBitmap(void)
	{
		return (bigCenterBitmap);
	}

	static inline bitMap getBigCenterBitmap(void)
	{
		return (centerBitmap | bigCenterBitmap);
	}

	static inline bitMap getBitmapSquareColor(const Color color)
	{
		assert(color < 2);
		return (BITMAP_COLOR[color]);
	}

	static inline bitMap getRankMask(const tSquare n)
	{
		assert( n < squareNumber );
		return (RANKMASK[n]);
	}

	static inline bitMap getFileMask(const tSquare n)
	{
		assert( n < squareNumber );
		return (FILEMASK[n]);
	}

/*	static inline bitMap getLine(const tSquare a,const tSquare b)
	{
		assert(a < squareNumber);
		assert(b < squareNumber);
		return LINES[a][b];
	}*/

	static inline bitMap getSquareBetween(const tSquare a,const tSquare b)
	{
		assert(a < squareNumber);
		assert(b < squareNumber);
		return (SQUARES_BETWEEN[a][b]);
	}

	/*	\brief return true if the 3 squares are aligned
		\author Marco Belli
		\version 1.0
		\date 08/11/2013
	*/
	static inline bool squaresAligned(const tSquare s1, const tSquare s2, const tSquare s3)
	{
		assert(s1 < squareNumber);
		assert(s2 < squareNumber);
		assert(s3 < squareNumber);
		return (LINES[s1][s2] & bitHelper::getBitmapFromSquare(s3));
	}

	static inline unsigned int getSquareDistance(const tSquare s1, const tSquare s2)
	{
		assert(s1 < squareNumber);
		assert(s2 < squareNumber);
		return (SQUARE_DISTANCE[s1][s2]);
	}

	static inline bitMap getIsolatedBitmap(const tSquare s1)
	{
		assert(s1 < squareNumber);
		return (ISOLATED_PAWN[s1]);
	}

	/*static inline bitMap isIsolated(const tSquare s1, const bitMap b)
	{
		assert(s1 < squareNumber);
		return (b & ISOLATED_PAWN[s1]);
	}*/

	static inline bitMap getPassedPawnBitmap(const Color c, const tSquare s1)
	{
		assert(s1 < squareNumber);
		assert(c <= black);
		return (PASSED_PAWN[c][s1]);

	}

	static inline bool isBitmapPreventingPassedPawn(const bitMap b, const Color c, const tSquare s1)
	{
		assert(s1 < squareNumber);
		assert(c <= black);
		return (b & PASSED_PAWN[c][s1]);

	}



	static inline bitMap getSquaresInFrontBitmap(const Color c, const tSquare s1)
	{
		assert(s1 < squareNumber);
		assert(c <= black);
		return (SQUARES_IN_FRONT_OF[c][s1]);

	}





};









//------------------------------------------------
//	inline functions
//------------------------------------------------


inline bitMap operator += (bitMap& b, const tSquare sq) { b = b | bitHelper::getBitmapFromSquare(sq); return (b); }


#endif /* DATA_H_ */
