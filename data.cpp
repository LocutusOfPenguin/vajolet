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

#include "data.h"
#include "bitmap.h"
#include <algorithm>

bitmap2 bitHelper::BITSET[squareNumber+1];
tSquare bitHelper::BOARDINDEX[8][8];

void bitHelper::initbitHelper()
{
	bitmap2 DIAGA1H8MASK[squareNumber];		//!< bitmask of a diagonal given a square on the diagonal
	bitmap2 DIAGA8H1MASK[squareNumber];		//!< bitmask of a diagonal given a square on the diagonal


	for(tSquare i = A1; i < squareNumber; i++)
	{
		BITSET[i] = (1ull) << i;
		BOARDINDEX[i%8][i/8] = i;
		DIAGA1H8MASK[i] = 0ull;
		DIAGA8H1MASK[i] = 0ull;

		for(tSquare square = A1; square < squareNumber; square++)
		{
			SQUARES_BETWEEN[square][i] = 0ull;
		}
	}
	BITSET[squareNone] = 0ull;


	centerBitmap = getBitmapFromSquare(E4) | getBitmapFromSquare(E5) | getBitmapFromSquare(D4) | getBitmapFromSquare(D5);
	bigCenterBitmap =
				getBitmapFromSquare(C6) | getBitmapFromSquare(D6) | getBitmapFromSquare(E6) | getBitmapFromSquare(F6)|
				getBitmapFromSquare(C5) | getBitmapFromSquare(C4) | getBitmapFromSquare(F5) | getBitmapFromSquare(F4)|
				getBitmapFromSquare(C3) | getBitmapFromSquare(D3) | getBitmapFromSquare(E3) | getBitmapFromSquare(F3);

	BITMAP_COLOR[0] = 0ull;
	BITMAP_COLOR[1] = 0ull;

	for(tSquare square = A1; square < squareNumber; square++)
	{
		if( bitHelper::getSquareColor( square ) == Color::black )
		{
			BITMAP_COLOR[ Color::black ] |= getBitmapFromSquare(square);
		}
		else
		{
			BITMAP_COLOR[ Color::white ] |= getBitmapFromSquare(square);
		}
	}

	for (int file = 0; file < 8; file++)
	{
		for (int rank = 0; rank < 8; rank++)
		{
			//===========================================================================
			//initialize 8-bit rank mask
			//===========================================================================

			RANKMASK[ bitHelper::getTsquareFromFileRank( file, rank ) ]  = getBitmapFromSquare( 0, rank ) | getBitmapFromSquare( 1, rank ) | getBitmapFromSquare( 2, rank ) | getBitmapFromSquare( 3, rank ) ;
			RANKMASK[ bitHelper::getTsquareFromFileRank( file, rank ) ] |= getBitmapFromSquare( 4, rank ) | getBitmapFromSquare( 5, rank ) | getBitmapFromSquare( 6, rank ) | getBitmapFromSquare( 7, rank ) ;

			//===========================================================================
			//initialize 8-bit file mask
			//===========================================================================
			FILEMASK[ getTsquareFromFileRank( file, rank ) ]  = getBitmapFromSquare( file, 0 ) | getBitmapFromSquare( file, 1 ) | getBitmapFromSquare( file, 2 ) | getBitmapFromSquare( file, 3 ) ;
			FILEMASK[ getTsquareFromFileRank( file, rank ) ] |= getBitmapFromSquare( file, 4 ) | getBitmapFromSquare( file, 5 ) | getBitmapFromSquare( file, 6 ) | getBitmapFromSquare( file, 7 ) ;

			//===========================================================================
			//Initialize 8-bit diagonal mask
			//===========================================================================
			int diaga8h1 = file + rank; // from 0 to 14, longest diagonal = 7
			if (diaga8h1 < 8)  // lower half, diagonals 0 to 7
			{
				for (int square = 0 ; square <= diaga8h1 ; square ++)
				{
					DIAGA8H1MASK[ getTsquareFromFileRank( file, rank ) ] |= getBitmapFromSquare( square, diaga8h1-square);
				}
			}
			else  // upper half, diagonals 8 to 14
			{
				for (int square = 0 ; square < 15 - diaga8h1 ; square ++)
				{
					DIAGA8H1MASK[ getTsquareFromFileRank( file, rank ) ] |= getBitmapFromSquare( diaga8h1 + square-7, 7-square );
				}
			}


			//===========================================================================
			//Initialize 8-bit diagonal mask
			//===========================================================================
			int diaga1h8 = file - rank; // from -7 to +7, longest diagonal = 0
			if (diaga1h8 > -1)  // lower half, diagonals 0 to 7
			{
				for (int square = 0 ; square <= 7 - diaga1h8 ; square ++)
				{
					DIAGA1H8MASK[ getTsquareFromFileRank( file, rank ) ] |= getBitmapFromSquare( diaga1h8 + square, square );
				}
			}
			else
			{
				for (int square = 0 ; square <= 7 + diaga1h8 ; square ++)
				{
					DIAGA1H8MASK[ getTsquareFromFileRank( file, rank ) ] |= getBitmapFromSquare( square, square - diaga1h8 );
				}
			}
		}
	}

	for(tSquare square = A1; square<squareNumber; square++)
	{
		for(tSquare i = A1; i<squareNumber; i++)
		{
			LINES[square][i] =0;
			SQUARES_BETWEEN[square][i] = 0;

			if(i == square)
			{
				continue;
			}

			if(getFile( square ) == getFile( i ) )
			{
				// stessa colonna

				LINES[square][i] = getFileMask(square);
				if(getRank( i ) > getRank( square ) ) // in salita
				{
					int temp = getRank( square ) + 1;
					while(temp < getRank( i ) )
					{
						SQUARES_BETWEEN[square][i] |= getBitmapFromSquare( getFile( square ) , temp );
						temp++;
					}
				}
				if(getRank( i )  < getRank( square ) ) // in discesa
				{
					int temp = getRank( square ) - 1;
					while(temp > getRank( i) )
					{
						SQUARES_BETWEEN[square][i] |= getBitmapFromSquare( getFile( square ), temp );
						temp--;
					}
				}
			}
			if(getRank( square ) == getRank( i) ) // stessa riga
			{
				LINES[square][i] = getRankMask(square);
				if( getFile( i ) > getFile( square ) ) // in salita
				{
					int temp = getFile( square)  + 1;
					while(temp < getFile( i ) )
					{
						SQUARES_BETWEEN[square][i] |= getBitmapFromSquare( temp, getRank( square ) );
						temp++;
					}
				}
				if( getFile( i ) < getFile( square ) ) // in discesa
				{
					int temp = getFile( square )  - 1;
					while(temp > getFile( i ) )
					{
						SQUARES_BETWEEN[square][i] |= getBitmapFromSquare( temp,getRank( square ) );
						temp--;
					}
				}
			}
			if( (DIAGA1H8MASK[square] & getBitmapFromSquare((tSquare)i) ).getBitmap())
			{
				LINES[square][i] = DIAGA1H8MASK[square].getBitmap();
				if( getFile( i ) > getFile( square ) ) // in salita
				{
					int temp = getFile( square ) + 1;
					int temp2 = getRank( square) + 1;
					while(temp < getFile( i ) )
					{
						SQUARES_BETWEEN[square][i] |= getBitmapFromSquare( temp, temp2 );
						temp++;
						temp2++;
					}
				}
				if( getFile( i ) < getFile( square ) ) // in discesa
				{
					int temp = getFile( square ) - 1;
					int temp2 = getRank( square ) - 1;
					while(temp > getFile( i ))
					{
						SQUARES_BETWEEN[square][i] |= getBitmapFromSquare( temp, temp2 );
						temp--;
						temp2--;
					}
				}
			}
			if( (DIAGA8H1MASK[square] & getBitmapFromSquare((tSquare)i) ).getBitmap())
			{
				LINES[square][i] = DIAGA8H1MASK[square].getBitmap();
				if(getFile( i ) > getFile( square ) ) // in salita
				{
					int temp = getFile( square ) + 1;
					int temp2 = getRank( square ) - 1;
					while(temp < getFile( i ) )
					{
						SQUARES_BETWEEN[square][i] |= getBitmapFromSquare( temp, temp2 );
						temp++;
						temp2--;
					}
				}
				if(getFile( i ) < getFile( square ) ) // in discesa
				{
					int temp = getFile( square ) - 1;
					int temp2 = getRank( square ) + 1;
					while(temp > getFile( i ))
					{
						SQUARES_BETWEEN[square][i] |= getBitmapFromSquare( temp, temp2 );
						temp--;
						temp2++;
					}
				}
			}
		}
	}

	for(tSquare square = A1; square < squareNumber; square++)
	{
		for(tSquare square2 = A1; square2 < squareNumber; square2++){

			SQUARE_DISTANCE[square][square2] = std::max(std::abs( getFile( square ) - getFile( square2 ) ), std::abs( getRank( square ) -  getRank( square2 ) ) );
		}
	}

	//////////////////////////////////////////////////
	//
	//	PAWN STRUCTURE DATA
	//
	//////////////////////////////////////////////////
	for(tSquare square = A1; square < squareNumber; square++)
	{
		ISOLATED_PAWN[square] = 0ull;
		int file = getFile( square );

		if(file>0)
		{
			ISOLATED_PAWN[square] |= getFileMask( getTsquareFromFileRank( file - 1, 0 ) );
		}
		if(file<7)
		{
			ISOLATED_PAWN[square] |= getFileMask( getTsquareFromFileRank( file + 1, 0 ) );
		}
	}

	for(tSquare square = A1; square < squareNumber; square++)
	{
		PASSED_PAWN[white][square] = 0ull;
		PASSED_PAWN[black][square] = 0ull;
		SQUARES_IN_FRONT_OF[white][square] = 0ull;
		SQUARES_IN_FRONT_OF[black][square] = 0ull;
		int file = bitHelper::getFile( square );
		int rank = bitHelper::getRank( square );

		for(int i = rank + 1; i < 8; i++)
		{
			if(file>0)
			{
				PASSED_PAWN[white][square] |= bitHelper::getBitmapFromSquare( file + ovest, i );
			}
			PASSED_PAWN[white][square] |= bitHelper::getBitmapFromSquare( file, i );
			SQUARES_IN_FRONT_OF[white][square] |= bitHelper::getBitmapFromSquare( file, i );
			if(file<7)
			{
				PASSED_PAWN[white][square] |= bitHelper::getBitmapFromSquare( file + est , i );
			}
		}

		for(int i = rank - 1; i >= 0; i--)
		{
			if(file>0)
			{
				PASSED_PAWN[black][square] |= bitHelper::getBitmapFromSquare( file + ovest, i );
			}
			PASSED_PAWN[black][square] |= bitHelper::getBitmapFromSquare( file, i );
			SQUARES_IN_FRONT_OF[black][square] |= bitHelper::getBitmapFromSquare( file, i );
			if(file<7)
			{
				PASSED_PAWN[black][square] |= bitHelper::getBitmapFromSquare( file + est, i );
			}
		}
	}


}
//--------------------------------------------------------------
//	global variables
//--------------------------------------------------------------


const int bitHelper::FILES[squareNumber] = {		//!< precalculated file from square number
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
};

const int bitHelper::RANKS[squareNumber] = {		//!< precalculated rank from square number
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
};

const Color bitHelper::SQUARE_COLOR[squareNumber]=
{
	Color::white, Color::black, Color::white, Color::black, Color::white, Color::black, Color::white, Color::black,
	Color::black, Color::white, Color::black, Color::white, Color::black, Color::white, Color::black, Color::white,
	Color::white, Color::black, Color::white, Color::black, Color::white, Color::black, Color::white, Color::black,
	Color::black, Color::white, Color::black, Color::white, Color::black, Color::white, Color::black, Color::white,
	Color::white, Color::black, Color::white, Color::black, Color::white, Color::black, Color::white, Color::black,
	Color::black, Color::white, Color::black, Color::white, Color::black, Color::white, Color::black, Color::white,
	Color::white, Color::black, Color::white, Color::black, Color::white, Color::black, Color::white, Color::black,
	Color::black, Color::white, Color::black, Color::white, Color::black, Color::white, Color::black, Color::white
};

bitmap2 bitHelper::BITMAP_COLOR[2];

bitmap2 bitHelper::RANKMASK[squareNumber];			//!< bitmask of a rank given a square on the rank
bitmap2 bitHelper::FILEMASK[squareNumber];			//!< bitmask of a file given a square on the rank


bitmap2 bitHelper::SQUARES_BETWEEN[squareNumber][squareNumber];		//bitmask with the squares btween 2 alinged squares, 0 otherwise
bitmap2 bitHelper::LINES[squareNumber][squareNumber];

bitmap2 bitHelper::ISOLATED_PAWN[squareNumber];
bitmap2 bitHelper::PASSED_PAWN[2][squareNumber];
bitmap2 bitHelper::SQUARES_IN_FRONT_OF[2][squareNumber];

unsigned int bitHelper::SQUARE_DISTANCE[squareNumber][squareNumber];

bitmap2 bitHelper::centerBitmap;
bitmap2 bitHelper::bigCenterBitmap;

