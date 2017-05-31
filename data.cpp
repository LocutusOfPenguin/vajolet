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
#include "bitops.h"
#include <algorithm>

bitMap bitHelper::BITSET[squareNumber+1];
tSquare bitHelper::BOARDINDEX[8][8];

void bitHelper::initbitHelper()
{
	for(tSquare i=(tSquare)0; i<squareNumber; i++)
	{
		BITSET[i] = (1ull) << i;
	}
	BITSET[squareNone]=0;

	for(tSquare i = (tSquare)0; i<squareNumber; i++)
	{
		BOARDINDEX[i%8][i/8] = i;
	}

	centerBitmap = getBitmapFromSquare(E4) | getBitmapFromSquare(E5) | getBitmapFromSquare(D4) | getBitmapFromSquare(D5);
	bigCenterBitmap =
				getBitmapFromSquare(C6) | getBitmapFromSquare(D6) | getBitmapFromSquare(E6) | getBitmapFromSquare(F6)|
				getBitmapFromSquare(C5) | getBitmapFromSquare(C4) | getBitmapFromSquare(F5) | getBitmapFromSquare(F4)|
				getBitmapFromSquare(C3) | getBitmapFromSquare(D3) | getBitmapFromSquare(E3) | getBitmapFromSquare(F3);

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

const int bitHelper::SQUARE_COLOR[squareNumber]=
{
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0,
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0,
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0,
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0
};

bitMap BITMAP_COLOR[2];

bitMap RANKMASK[squareNumber];			//!< bitmask of a rank given a square on the rank
bitMap FILEMASK[squareNumber];			//!< bitmask of a file given a square on the rank

bitMap DIAGA1H8MASK[squareNumber];		//!< bitmask of a diagonal given a square on the diagonal
bitMap DIAGA8H1MASK[squareNumber];		//!< bitmask of a diagonal given a square on the diagonal
bitMap SQUARES_BETWEEN[squareNumber][squareNumber];		//bitmask with the squares btween 2 alinged squares, 0 otherwise
bitMap LINES[squareNumber][squareNumber];

bitMap ISOLATED_PAWN[squareNumber];
bitMap PASSED_PAWN[2][squareNumber];
bitMap SQUARES_IN_FRONT_OF[2][squareNumber];

int SQUARE_DISTANCE[squareNumber][squareNumber];

bitMap bitHelper::centerBitmap;
bitMap bitHelper::bigCenterBitmap;

//--------------------------------------------------------------
//	function bodies
//--------------------------------------------------------------

/*	\brief init global data structures
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
void initData(void)
{

	for(tSquare i = (tSquare)0; i<squareNumber; i++)
	{
		DIAGA1H8MASK[i] = 0;
		DIAGA8H1MASK[i] = 0;
	}





	for (int file = 0; file < 8; file++)
	{
		for (int rank = 0; rank < 8; rank++)
		{
			//===========================================================================
			//initialize 8-bit rank mask
			//===========================================================================

			RANKMASK[ bitHelper::getTsquareFromFileRank( file, rank ) ]  = bitHelper::getBitmapFromSquare( 0, rank ) | bitHelper::getBitmapFromSquare( 1, rank ) | bitHelper::getBitmapFromSquare( 2, rank ) | bitHelper::getBitmapFromSquare( 3, rank ) ;
			RANKMASK[ bitHelper::getTsquareFromFileRank( file, rank ) ] |= bitHelper::getBitmapFromSquare( 4, rank ) | bitHelper::getBitmapFromSquare( 5, rank ) | bitHelper::getBitmapFromSquare( 6, rank ) | bitHelper::getBitmapFromSquare( 7, rank ) ;

			//===========================================================================
			//initialize 8-bit file mask
			//===========================================================================
			FILEMASK[ bitHelper::getTsquareFromFileRank( file, rank ) ]  = bitHelper::getBitmapFromSquare( file, 0 ) | bitHelper::getBitmapFromSquare( file, 1 ) | bitHelper::getBitmapFromSquare( file, 2 ) | bitHelper::getBitmapFromSquare( file, 3 ) ;
			FILEMASK[ bitHelper::getTsquareFromFileRank( file, rank ) ] |= bitHelper::getBitmapFromSquare( file, 4 ) | bitHelper::getBitmapFromSquare( file, 5 ) | bitHelper::getBitmapFromSquare( file, 6 ) | bitHelper::getBitmapFromSquare( file, 7 ) ;

			//===========================================================================
			//Initialize 8-bit diagonal mask
			//===========================================================================
			int diaga8h1 = file + rank; // from 0 to 14, longest diagonal = 7
			if (diaga8h1 < 8)  // lower half, diagonals 0 to 7
			{
				for (int square = 0 ; square <= diaga8h1 ; square ++)
				{
					DIAGA8H1MASK[ bitHelper::getTsquareFromFileRank( file, rank ) ] |= bitHelper::getBitmapFromSquare( square, diaga8h1-square);
				}
			}
			else  // upper half, diagonals 8 to 14
			{
				for (int square = 0 ; square < 15 - diaga8h1 ; square ++)
				{
					DIAGA8H1MASK[ bitHelper::getTsquareFromFileRank( file, rank ) ] |= bitHelper::getBitmapFromSquare( diaga8h1 + square-7, 7-square );
				}
			}


			//===========================================================================
			//Initialize 8-bit diagonal mask, used in the movegenerator (see movegen.ccp)
			//===========================================================================
			int diaga1h8 = file - rank; // from -7 to +7, longest diagonal = 0
			if (diaga1h8 > -1)  // lower half, diagonals 0 to 7
			{
				for (int square = 0 ; square <= 7 - diaga1h8 ; square ++)
				{
					DIAGA1H8MASK[ bitHelper::getTsquareFromFileRank( file, rank ) ] |= bitHelper::getBitmapFromSquare( diaga1h8 + square, square );
				}
			}
			else
			{
				for (int square = 0 ; square <= 7 + diaga1h8 ; square ++)
				{
					DIAGA1H8MASK[ bitHelper::getTsquareFromFileRank( file, rank ) ] |= bitHelper::getBitmapFromSquare( square, square - diaga1h8 );
				}
			}

		}
	}

	for(int square=0; square<squareNumber; square++)
	{
		for(int i=0; i<squareNumber; i++)
		{
			SQUARES_BETWEEN[square][i] = 0;
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

			if(bitHelper::getFile( square ) == bitHelper::getFile( i ) )
			{
				// stessa colonna

				LINES[square][i] = FILEMASK[square];
				if(bitHelper::getRank( i ) > bitHelper::getRank( square ) ) // in salita
				{
					int temp = bitHelper::getRank( square ) + 1;
					while(temp < bitHelper::getRank( i ) )
					{
						SQUARES_BETWEEN[square][i] |= bitHelper::getBitmapFromSquare( bitHelper::getFile( square ) , temp );
						temp++;
					}
				}
				if(bitHelper::getRank( i )  < bitHelper::getRank( square ) ) // in discesa
				{
					int temp = bitHelper::getRank( square ) - 1;
					while(temp > bitHelper::getRank( i) )
					{
						SQUARES_BETWEEN[square][i] |= bitHelper::getBitmapFromSquare( bitHelper::getFile( square ), temp );
						temp--;
					}
				}
			}
			if(bitHelper::getRank( square ) == bitHelper::getRank( i) ) // stessa riga
			{
				LINES[square][i] = RANKMASK[square];
				if( bitHelper::getFile( i ) > bitHelper::getFile( square ) ) // in salita
				{
					int temp = bitHelper::getFile( square)  + 1;
					while(temp < bitHelper::getFile( i ) )
					{
						SQUARES_BETWEEN[square][i] |= bitHelper::getBitmapFromSquare( temp, bitHelper::getRank( square ) );
						temp++;
					}
				}
				if( bitHelper::getFile( i ) < bitHelper::getFile( square ) ) // in discesa
				{
					int temp = bitHelper::getFile( square )  - 1;
					while(temp > bitHelper::getFile( i ) )
					{
						SQUARES_BETWEEN[square][i] |= bitHelper::getBitmapFromSquare( temp,bitHelper::getRank( square ) );
						temp--;
					}
				}
			}
			if(DIAGA1H8MASK[square] & bitHelper::getBitmapFromSquare((tSquare)i))
			{
				LINES[square][i] = DIAGA1H8MASK[square];
				if( bitHelper::getFile( i ) > bitHelper::getFile( square ) ) // in salita
				{
					int temp = bitHelper::getFile( square ) + 1;
					int temp2 = bitHelper::getRank( square) + 1;
					while(temp < bitHelper::getFile( i ) )
					{
						SQUARES_BETWEEN[square][i] |= bitHelper::getBitmapFromSquare( temp, temp2 );
						temp++;
						temp2++;
					}
				}
				if( bitHelper::getFile( i ) < bitHelper::getFile( square ) ) // in discesa
				{
					int temp = bitHelper::getFile( square ) - 1;
					int temp2 = bitHelper::getRank( square ) - 1;
					while(temp > bitHelper::getFile( i ))
					{
						SQUARES_BETWEEN[square][i] |= bitHelper::getBitmapFromSquare( temp, temp2 );
						temp--;
						temp2--;
					}
				}
			}
			if(DIAGA8H1MASK[square] & bitHelper::getBitmapFromSquare((tSquare)i) )
			{
				LINES[square][i] = DIAGA8H1MASK[square];
				if(bitHelper::getFile( i ) > bitHelper::getFile( square ) ) // in salita
				{
					int temp = bitHelper::getFile( square ) + 1;
					int temp2 = bitHelper::getRank( square ) - 1;
					while(temp < bitHelper::getFile( i ) )
					{
						SQUARES_BETWEEN[square][i] |= bitHelper::getBitmapFromSquare( temp, temp2 );
						temp++;
						temp2--;
					}
				}
				if(bitHelper::getFile( i ) < bitHelper::getFile( square ) ) // in discesa
				{
					int temp = bitHelper::getFile( square ) - 1;
					int temp2 = bitHelper::getRank( square ) + 1;
					while(temp > bitHelper::getFile( i ))
					{
						SQUARES_BETWEEN[square][i] |= bitHelper::getBitmapFromSquare( temp, temp2 );
						temp--;
						temp2++;
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////
	//
	//	PAWN STRUCTURE DATA
	//
	//////////////////////////////////////////////////
	for(tSquare square = A1; square < squareNumber; square++)
	{
		ISOLATED_PAWN[square] = 0;
		int file = bitHelper::getFile( square );

		if(file>0)
		{
			ISOLATED_PAWN[square] |= FILEMASK[ bitHelper::getTsquareFromFileRank( file - 1, 0 ) ];
		}
		if(file<7)
		{
			ISOLATED_PAWN[square] |= FILEMASK[ bitHelper::getTsquareFromFileRank( file + 1, 0 ) ];
		}
	}

	for(tSquare square = A1; square < squareNumber; square++)
	{
		PASSED_PAWN[0][square] = 0;
		PASSED_PAWN[1][square] = 0;
		SQUARES_IN_FRONT_OF[0][square] = 0;
		SQUARES_IN_FRONT_OF[1][square] = 0;
		int file = bitHelper::getFile( square );
		int rank = bitHelper::getRank( square );

		for(int i = rank + 1; i < 8; i++)
		{
			if(file>0)
			{
				PASSED_PAWN[0][square] |= bitHelper::getBitmapFromSquare( file + ovest, i );
			}
			PASSED_PAWN[0][square] |= bitHelper::getBitmapFromSquare( file, i );
			SQUARES_IN_FRONT_OF[0][square] |= bitHelper::getBitmapFromSquare( file, i );
			if(file<7)
			{
				PASSED_PAWN[0][square] |= bitHelper::getBitmapFromSquare( file + est , i );
			}
		}

		for(int i = rank - 1; i >= 0; i--)
		{
			if(file>0)
			{
				PASSED_PAWN[1][square] |= bitHelper::getBitmapFromSquare( file + ovest, i );
			}
			PASSED_PAWN[1][square] |= bitHelper::getBitmapFromSquare( file, i );
			SQUARES_IN_FRONT_OF[1][square] |= bitHelper::getBitmapFromSquare( file, i );
			if(file<7)
			{
				PASSED_PAWN[1][square] |= bitHelper::getBitmapFromSquare( file + est, i );
			}
		}
	}

	for(tSquare square = A1; square < squareNumber; square++)
	{
		for(tSquare square2 = A1; square2 < squareNumber; square2++){

			SQUARE_DISTANCE[square][square2] = std::max(std::abs(bitHelper::getFile( square ) - bitHelper::getFile( square2 ) ), std::abs(bitHelper::getRank( square ) - bitHelper::getRank( square2 ) ) );
		}
	}

	BITMAP_COLOR[0] = 0;
	BITMAP_COLOR[1] = 0;

	for(tSquare square = A1; square < squareNumber; square++)
	{
		if( bitHelper::getSquareColor( square ) )
		{
			BITMAP_COLOR[1] |= bitHelper::getBitmapFromSquare(square);
		}
		else
		{
			BITMAP_COLOR[0] |= bitHelper::getBitmapFromSquare(square);
		}
	}
}

