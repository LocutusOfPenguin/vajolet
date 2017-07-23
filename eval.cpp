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

#include <utility>

#include <iomanip>
#include "data.h"
#include "position.h"
#include "move.h"
#include "bitops.h"
#include "movegen.h"
#include "eval.h"





const int KingExposed[] = {
     2,  0,  2,  5,  5,  2,  0,  2,
     2,  2,  4,  8,  8,  4,  2,  2,
     7, 10, 12, 12, 12, 12, 10,  7,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15
  };

//------------------------------------------------
//	MOBILITY BONUS
//------------------------------------------------
simdScore queenMobilityPars={5,5,20,335};
simdScore rookMobilityPars={5,5,215,230};
simdScore bishopMobilityPars={4,4,283,318};
simdScore knightMobilityPars={3,3,280,220};
simdScore mobilityBonus[Position::separationBitmap][32] =
{
		{0},
		{0},
		// queen
		{
				{-2100,-2675},{-80,-1340}, {-60,-1005}, {-40,-670}, {-20,-335}, {0,0},      {20,335},   {40,670},
				{60,1005},    {80,1340},   {100,1675},  {120,2010}, {140,2345}, {160,2680}, {180,3015}, {200,3350},
				{220,3685},   {240,4020},  {260,4355},  {280,4690}, {300,5025}, {320,5360}, {340,5695}, {360,6030},
				{380,6365},   {400,6700},  {420,7035},  {440,7370}, {460,7705}, {480,8040}, {500,8375}, {520,8710}
		},
		// rook
		{
				{-3075, -3150}, {-860, -920}, {-645, -690}, {-430, -460}, {-215, -230},{0,0},        {215, 230},   {430, 460},
				{645, 690},    {860, 920},   {1075, 1150}, {1290, 1380}, {1505, 1610}, {1720, 1840}, {1935, 2070}, {2150, 2300}
		},
		// bishop
		{
				{-3132, -3272}, {-849, -954}, {-566, -636}, {-283, -318}, {0, 0},       {283, 318},    {566, 636},   {849, 954},
				{1132, 1272},   {1415, 1590}, {1698, 1908}, {1981, 2226}, {2264, 2544}, {2547, 2862 }, {2830, 3180}, {3113, 3498}
		},
		// knight
		{
				{-4000, -3660}, {-1000, -440}, {-500, -220}, {0, 0}, {280, 220}, {600, 440}, {900, 660}, {1300, 880}, {1400, 1100}
		},
		{0},
		{0}

};


//------------------------------------------------
//	PAWN Bonus/Penalties
//------------------------------------------------
simdScore isolatedPawnPenalty={533,2410,0,0};
simdScore isolatedPawnPenaltyOpp={100,300,0,0};
simdScore doubledPawnPenalty={290,1950,0,0};
simdScore backwardPawnPenalty={1120,2680,0,0};
simdScore chainedPawnBonus={540,180,0,0};
simdScore passedPawnFileAHPenalty = {-1030,720,0,0};
simdScore passedPawnSupportedBonus = {1060,-450,0,0};
simdScore candidateBonus = {110,1330,0,0};
simdScore passedPawnBonus = {460,840,0,0};
simdScore passedPawnUnsafeSquares ={-10,70,0,0};
simdScore passedPawnBlockedSquares ={150,370,0,0};
simdScore passedPawnDefendedSquares = {150,240,0,0};
simdScore passedPawnDefendedBlockingSquare = {260,170,0,0};
simdScore unstoppablePassed = {0,2000,0,0};
simdScore rookBehindPassedPawn = {0,10,0,0};
simdScore EnemyRookBehindPassedPawn = {0,10,0,0};

simdScore holesPenalty={100,260,0,0};
simdScore pawnCenterControl={70,120,0,0};
simdScore pawnBigCenterControl={360,-670,0,0};


simdScore pieceCoordination={280,160,0,0};

simdScore piecesCenterControl={170,160,0,0};
simdScore piecesBigCenterControl={120,50,0,0};

simdScore rookOn7Bonus={3700,1800,0,0};
simdScore rookOnPawns={-600,1300,0,0};
simdScore queenOn7Bonus={-3200,7000,0,0};
simdScore queenOnPawns={-1000,3000,0,0};
simdScore rookOnOpen={2000,500,0,0};
simdScore rookOnSemi={500,1100,0,0};
simdScore rookTrapped = {300,0,0,0};
simdScore rookTrappedKingWithoutCastling = {300,0,0,0};

simdScore knightOnOutpost= {380,0,0,0};
simdScore knightOnOutpostSupported= {100,1290,0,0};
simdScore knightOnHole= {1610,1190,0,0};
simdScore KnightAttackingWeakPawn= {300,300,0,0};

simdScore bishopOnOutpost= {-1020,810,0,0};
simdScore bishopOnOutpostSupported= {3600,270,0,0};
simdScore bishopOnHole= {590,-730,0,0};
simdScore badBishop1= {-200,0,0,0};
simdScore badBishop2= {0,1530,0,0};

simdScore tempo= {530,480,0,0};
simdScore bishopPair ={3260,4690,0,0};

simdScore ownKingNearPassedPawn={0,150,0,0};
simdScore enemyKingNearPassedPawn={10,240,0,0};

simdScore spaceBonus={600,200,0,0};
simdScore undefendedMinorPenalty = {756,354,0,0};

simdScore attackedByPawnPenalty[Position::separationBitmap]=
{	{0,0,0,0},
	{0,0,0,0},//king
	{1000,-7800,0,0},//queen
	{2225,1627,0,0},//rook
	{3200,350,0,0},//bishop
	{2900,2750,0,0},//knight
	{0,0,0,0},//pawn
	{0,0,0,0},
};

simdScore weakPiecePenalty[Position::separationBitmap][Position::separationBitmap]=
{	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},//king
	{{0,0,0,0},{0,0,0,0},	{-460,300,0,0},	{3812,120,0,0},{3580,600,0,0},{1420,-4400,0,0},{1400,2700,0,0},	{0,0,0,0}},//queen
	{{0,0,0,0},{0,0,0,0},	{1200,3200,0,0},	{3220,2420,0,0},{4900,3100,0,0},{3400,1700,0,0},{1000,100,0,0},	{0,0,0,0}},//rook
	{{0,0,0,0},{0,0,0,0},	{2400,4000,0,0},	{2820,1850,0,0},{400,2800,0,0},{3100,4400,0,0},{-900,4690,0,0},	{0,0,0,0}},//bishop
	{{0,0,0,0},{0,0,0,0},	{1300,400,0,0},	{3121,3147,0,0},{1723,2917,0,0},{2223,1117,0,0},{-810,554,0,0},	{0,0,0,0}},//knight
	{{0,0,0,0},{0,0,0,0},	{50,40,0,0},		{60,50,0,0},	{100,80,0,0},	{150,90,0,0},	{200,170,0,0},	{0,0,0,0}},//pawn
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}}
//						king				queen						rook					bishop					knight					pawn
};
//------------------------------------------------
//king safety
//------------------------------------------------
const unsigned int KingAttackWeights[] = { 0, 0, 5, 3, 2, 2 };
Score kingShieldBonus= 2400;
Score kingFarShieldBonus= 1800;
Score kingStormBonus= 80;
simdScore kingSafetyBonus={93,-5,0,0};
simdScore kingSafetyScaling={310,0,0,0};
simdScore KingSafetyMaxAttack={93,0,0,0};
simdScore KingSafetyLinearCoefficent={5,0,0,0};
simdScore KingSafetyMaxResult={1000,0,0,0};
//------------------------------------------------


simdScore queenVsRook2MinorsImbalance={20000,20000,0,0};


//---------------------------------------------
//	MATERIAL KEYS
//---------------------------------------------




std::unordered_map<U64, Position::materialStruct> Position::materialKeyMap;

bool Position::evalKBPvsK(Score& res)
{
	Color Pcolor = getBitmap(whitePawns) ? white : black;
	tSquare pawnSquare;
	tSquare bishopSquare;
	if(Pcolor == white)
	{
		pawnSquare = getSquareOfThePiece(whitePawns);
		int pawnFile = bitHelper::getFile( pawnSquare );
		if( pawnFile ==0 || pawnFile ==7 )
		{
			bishopSquare = getSquareOfThePiece(whiteBishops);
			if( bitHelper::getSquareColor( bitHelper::getTsquareFromFileRank( pawnFile, 7) ) != bitHelper::getSquareColor( bishopSquare ) )
			{
				tSquare kingSquare = getSquareOfThePiece(blackKing);
				if(bitHelper::getRank( kingSquare ) >= 6  && abs( pawnFile - bitHelper::getFile( kingSquare ) ) <= 1 )
				{
					res = 0;
					return true;
				}
			}
		}
	}
	else
	{
		pawnSquare = getSquareOfThePiece(blackPawns);
		int pawnFile = bitHelper::getFile( pawnSquare );
		if( pawnFile==0 || pawnFile == 7 )
		{
			bishopSquare = getSquareOfThePiece(blackBishops);
			if( bitHelper::getSquareColor( bitHelper::getTsquareFromFileRank( pawnFile, 0) ) != bitHelper::getSquareColor( bishopSquare ) )
			{
				tSquare kingSquare = getSquareOfThePiece(whiteKing);
				if( bitHelper::getRank( kingSquare ) <= 1  && abs(pawnFile - bitHelper::getFile( kingSquare ) ) <= 1)
				{
					res = 0;
					return true;
				}
			}
		}
	}
	return false;

}

bool Position::evalKQvsKP(Score& res)
{
	Color Pcolor = getBitmap(whitePawns) ? white  :black;
	tSquare pawnSquare;
	tSquare winningKingSquare;
	tSquare losingKingSquare;


	if(Pcolor == white)
	{
		pawnSquare = getSquareOfThePiece(whitePawns);
		winningKingSquare = getSquareOfThePiece(blackKing);
		losingKingSquare = getSquareOfThePiece(whiteKing);

		int pawnFile = bitHelper::getFile( pawnSquare );
		int pawnRank = bitHelper::getRank( pawnSquare );
		res = -100 * ( 7 - bitHelper::getSquareDistance( winningKingSquare, losingKingSquare ) );

		if(
				pawnRank != 6
				|| bitHelper::getSquareDistance( losingKingSquare, pawnSquare ) != 1
				|| (pawnFile == 1 || pawnFile == 3 || pawnFile == 4 || pawnFile == 6) )
		{
			res -= 90000;
		}
		return true;

	}
	else
	{
		pawnSquare = getSquareOfThePiece(blackPawns);
		winningKingSquare = getSquareOfThePiece(whiteKing);
		losingKingSquare = getSquareOfThePiece(blackKing);

		int pawnFile = bitHelper::getFile( pawnSquare );
		int pawnRank = bitHelper::getRank( pawnSquare );
		res = 100 * ( 7 - bitHelper::getSquareDistance( winningKingSquare, losingKingSquare ) );

		if(
				pawnRank != 1
				|| bitHelper::getSquareDistance( losingKingSquare, pawnSquare ) != 1
				|| (pawnFile == 1 || pawnFile == 3 || pawnFile == 4 || pawnFile == 6) )
		{
			res += 90000;
		}
		return true;

	}
	return false;

}


bool Position::evalKRPvsKr(Score& res)
{
	Color Pcolor = getBitmap(whitePawns) ? white : black;
	tSquare pawnSquare;
	if( Pcolor == white )
	{
		pawnSquare = getSquareOfThePiece(whitePawns);
		if(	bitHelper::getFile( pawnSquare)  == bitHelper::getFile( getSquareOfThePiece(blackKing) )
		    && bitHelper::getRank( pawnSquare ) <= 6
		    && bitHelper::getRank( pawnSquare ) < bitHelper::getRank( getSquareOfThePiece(blackKing) )
		)
		{
			res = 128;
			return true;
		}
	}
	else
	{
		pawnSquare = getSquareOfThePiece(blackPawns);
		if(	bitHelper::getFile( pawnSquare ) == bitHelper::getFile( getSquareOfThePiece(whiteKing) )
			&& bitHelper::getRank( pawnSquare ) >= 1
			&& bitHelper::getRank( pawnSquare ) > bitHelper::getRank( getSquareOfThePiece(whiteKing) )
		)
		{
			res=128;
			return true;
		}

	}
	return false;

}

bool Position::evalKBNvsK( Score& res)
{
	Color color = getBitmap(whiteBishops) ? white : black;
	tSquare bishopSquare;
	tSquare kingSquare;
	tSquare enemySquare;
	tSquare mateSquare1, mateSquare2;
	int mul = 1;
	if(color == white)
	{
		mul = 1;
		bishopSquare = getSquareOfThePiece(whiteBishops);
		kingSquare = getSquareOfThePiece(whiteKing);
		enemySquare = getSquareOfThePiece(blackKing);
	}
	else
	{
		mul = -1;
		bishopSquare = getSquareOfThePiece(blackBishops);
		kingSquare = getSquareOfThePiece(blackKing);
		enemySquare = getSquareOfThePiece(whiteKing);
	}

	Color mateColor = bitHelper::getSquareColor( bishopSquare );
	if(mateColor == Color::white )
	{
		mateSquare1 = A1;
		mateSquare2 = H8;
	}
	else
	{
		mateSquare1 = A8;
		mateSquare2 = H1;
	}

	res = SCORE_KNOWN_WIN + 50000;

	res -= 5 * bitHelper::getSquareDistance( enemySquare, kingSquare );// devo tenere il re vicino
	res -= 10 * std::min( bitHelper::getSquareDistance( enemySquare, mateSquare1 ), bitHelper::getSquareDistance( enemySquare,mateSquare2 ) );// devo portare il re avversario nel giusto angolo

	res *=mul;
	return true;

}

bool Position::evalKQvsK(Score& res)
{
	Color color = getBitmap(whiteQueens) ? white : black;
	tSquare queenSquare;
	tSquare kingSquare;
	tSquare enemySquare;

	int mul = 1;
	if(color == white)
	{
		mul = 1;
		queenSquare = getSquareOfThePiece(whiteQueens);
		kingSquare = getSquareOfThePiece(whiteKing);
		enemySquare = getSquareOfThePiece(blackKing);
	}
	else
	{
		mul = -1;
		queenSquare = getSquareOfThePiece(blackQueens);
		kingSquare = getSquareOfThePiece(blackKing);
		enemySquare = getSquareOfThePiece(whiteKing);
	}

	res = SCORE_KNOWN_WIN + 40000;
	res -= 5 * bitHelper::getSquareDistance( enemySquare, kingSquare );// devo tenere il re vicino
	res -= 5 * bitHelper::getSquareDistance( enemySquare, queenSquare );// devo portare il re avversario nel giusto angolo

	res *= mul;
	return true;

}

bool Position::evalKRvsK(Score& res)
{
	Color color = getBitmap(whiteRooks) ? white : black;
	tSquare rookSquare;
	tSquare kingSquare;
	tSquare enemySquare;

	int mul = 1;
	if(color == white)
	{
		mul = 1;
		rookSquare = getSquareOfThePiece(whiteRooks);
		kingSquare = getSquareOfThePiece(whiteKing);
		enemySquare = getSquareOfThePiece(blackKing);
	}
	else
	{
		mul = -1;
		rookSquare = getSquareOfThePiece(blackRooks);
		kingSquare = getSquareOfThePiece(blackKing);
		enemySquare = getSquareOfThePiece(whiteKing);
	}

	res = SCORE_KNOWN_WIN + 30000;
	res -= 5 * bitHelper::getSquareDistance( enemySquare, kingSquare );// devo tenere il re vicino
	res -= 5 * bitHelper::getSquareDistance( enemySquare, rookSquare );// devo portare il re avversario nel giusto angolo

	res *= mul;
	return true;

}

bool Position::kingsDirectOpposition()
{
	if(
			(getSquareOfThePiece(whiteKing) + 16 == getSquareOfThePiece(blackKing) )
			/*||
			(getSquareOfThePiece(whiteKing) == getSquareOfThePiece(blackKing) +16 )*/
	)
	{
		return true;
	}
	return false;

}

bool Position::evalKPvsK(Score& res)
{
	Color color = getBitmap(whitePawns) ? white : black;
	tSquare pawnSquare;
	tSquare kingSquare;
	tSquare enemySquare;

	if(color == white)
	{
		pawnSquare = getSquareOfThePiece(whitePawns);
		kingSquare = getSquareOfThePiece(whiteKing);
		enemySquare = getSquareOfThePiece(blackKing);


		tSquare promotionSquare = bitHelper::getTsquareFromFileRank( bitHelper::getFile( pawnSquare ), 7);
		const int relativeRank = bitHelper::getRank( pawnSquare );
		// Rule of the square
		if ( std::min( 5, (int)(7- relativeRank)) <  std::max( (int) bitHelper::getSquareDistance( enemySquare, promotionSquare ) - (getNextTurn() == whiteTurn? 0 : 1) , 0) )
		{
			res = SCORE_KNOWN_WIN + relativeRank;
			return true;
		}
		if( bitHelper::getFile( pawnSquare ) != 0 && bitHelper::getFile( pawnSquare ) != 7)
		{

			if(bitHelper::getSquareDistance( enemySquare, pawnSquare ) >= 2 || getNextTurn() == Position::whiteTurn )
			{
				//winning king on a key square
				if(relativeRank < 4)
				{
					if(kingSquare >= pawnSquare + 15 && kingSquare <= pawnSquare + 17 )
					{
						res = SCORE_KNOWN_WIN + relativeRank;
						return true;
					}
				}
				else if(relativeRank < 6)
				{
					if((kingSquare >= pawnSquare + 15 && kingSquare <= pawnSquare + 17) || (kingSquare >= pawnSquare + 7 && kingSquare <= pawnSquare + 9))
					{
						res = SCORE_KNOWN_WIN + relativeRank;
						return true;
					}
				}
				else{

					if((kingSquare >= pawnSquare - 1 && kingSquare <= pawnSquare + 1 ) || (kingSquare >= pawnSquare + 7 && kingSquare <= pawnSquare + 9))
					{
						res = SCORE_KNOWN_WIN + relativeRank;
						return true;
					}

				}

				// 3 rules for winning, if  conditions are met -> it's won
				unsigned int count = 0;
				if(kingSquare == pawnSquare + 8) count++;
				if(getNextTurn() == blackTurn && kingsDirectOpposition()) count++;
				if(bitHelper::getRank( kingSquare ) == 5) count++;

				if(count > 1)
				{
					res = SCORE_KNOWN_WIN + relativeRank;
					return true;
				}

			}
			//draw rule
			if((enemySquare==pawnSquare+8) || (enemySquare==pawnSquare+16 && bitHelper::getRank( enemySquare ) != 7 ) )
			{
				res = 0;
				return true;
			}
		}
		else
		{
			//ROOKS PAWN
			if( abs( bitHelper::getFile( enemySquare) - bitHelper::getFile( pawnSquare) ) <= 1  && bitHelper::getRank( enemySquare ) > 5 )
			{
				res = 0;
				return true;
			}


		}
	}
	else{
		pawnSquare = getSquareOfThePiece(blackPawns);
		kingSquare = getSquareOfThePiece(blackKing);
		enemySquare = getSquareOfThePiece(whiteKing);



		tSquare promotionSquare = bitHelper::getTsquareFromFileRank ( bitHelper::getFile( pawnSquare) , 0 );
		const int relativeRank = 7 - bitHelper::getRank( pawnSquare );
		// Rule of the square
		if ( std::min( 5, (int)( 7 - relativeRank)) <  std::max( (int)bitHelper::getSquareDistance( enemySquare, promotionSquare ) - (getNextTurn() == blackTurn ? 0 : 1 ), 0) )
		{
			res = -SCORE_KNOWN_WIN - relativeRank;
			return true;
		}
		if( bitHelper::getFile( pawnSquare ) != 0 && bitHelper::getFile( pawnSquare ) != 7)
		{
			if( bitHelper::getSquareDistance( enemySquare, pawnSquare ) >= 2 || getNextTurn() == blackTurn)
			{
				//winning king on a key square
				if(relativeRank < 4)
				{
					if(kingSquare >= pawnSquare - 17 && kingSquare <= pawnSquare - 15 )
					{
						res = -SCORE_KNOWN_WIN - relativeRank;
						return true;
					}
				}
				else if(relativeRank < 6)
				{
					if((kingSquare >= pawnSquare - 17 && kingSquare <= pawnSquare - 15) || (kingSquare >= pawnSquare - 9 && kingSquare <= pawnSquare - 7 ) )
					{
						res = -SCORE_KNOWN_WIN - relativeRank;
						return true;
					}
				}
				else{

					if((kingSquare >= pawnSquare - 1 && kingSquare <= pawnSquare + 1) || (kingSquare >= pawnSquare - 9 && kingSquare <= pawnSquare - 7))
					{
						res = -SCORE_KNOWN_WIN - relativeRank;
						return true;
					}
				}
				// 3 rules for winning, if  conditions are met -> it's won
				unsigned int count = 0;
				if(kingSquare == pawnSquare - 8) count++;
				if(getNextTurn() == whiteTurn && kingsDirectOpposition()) count++;
				if(bitHelper::getRank( kingSquare ) == 2) count++;

				if(count > 1)
				{
					res = -SCORE_KNOWN_WIN - relativeRank;
					return true;
				}
			}
			//draw rule
			if((enemySquare == pawnSquare - 8) || (enemySquare == pawnSquare - 16 && bitHelper::getRank( enemySquare ) != 0) )
			{
				res = 0;
				return true;
			}
		}
		else
		{
			//ROOKS PAWN
			if( abs( bitHelper::getFile( enemySquare) - bitHelper::getFile( pawnSquare ) ) <= 1  && bitHelper::getRank( enemySquare ) < 2)
			{
				res = 0;
				return true;
			}


		}
	}


	return false;
}


bool Position::evalOppositeBishopEndgame(Score& res)
{
	if( bitHelper::getSquareColor( getSquareOfThePiece(blackBishops) ) != bitHelper::getSquareColor( getSquareOfThePiece(whiteBishops) ) )
	{
		unsigned int pawnCount = 0;
		int pawnDifference = 0;
		unsigned int freePawn = 0;

		bitMap pawns= getBitmap(whitePawns);
		while(pawns)
		{
			pawnCount++;
			pawnDifference++;
			tSquare pawn = iterateBit(pawns);
			if( !bitHelper::isBitmapPreventingPassedPawn(getBitmap(blackPawns), white, pawn ) )
			{
				if(!(Movegen::getBishopPseudoAttack(getSquareOfThePiece(blackBishops)) & bitHelper::getSquaresInFrontBitmap( white, pawn ) ) )
				{
					res = 256;
					freePawn++;
				}
			}
		}

		pawns= getBitmap(blackPawns);
		while(pawns)
		{
			pawnCount++;
			pawnDifference--;

			tSquare pawn = iterateBit(pawns);
			if( !bitHelper::isBitmapPreventingPassedPawn(getBitmap(whitePawns), black, pawn ) )
			{
				if(!(Movegen::getBishopPseudoAttack(getSquareOfThePiece(whiteBishops)) & bitHelper::getSquaresInFrontBitmap( black, pawn ) ) )
				{
					freePawn++;
					res = 256;
				}
			}
		}

		if(freePawn == 0 && std::abs(pawnDifference) <= 1 )
		{
			res = std::min(20 + pawnCount * 25, (unsigned int)256);
			return true;
		}

	}
	return false;

}

bool Position::evalKRvsKm(Score& res)
{
	res = 64;
	return true;
}

bool Position::evalKNNvsK(Score& res)
{
	res = 10;
	return true;
}

void Position::initMaterialKeys(void)
{
	/*---------------------------------------------
	 *
	 * K vs K		->	draw
	 * km vs k		->	draw
	 * km vs km		->	draw
	 * kmm vs km	->	draw
	 *
	 * kbp vs k		->	probable draw on the rook file
	 *
	 * kb vs kpawns	-> bishop cannot win
	 * kn vs kpawns	-> bishop cannot win
	 *
	 * kbn vs k		-> win
	 * opposite bishop endgame -> look drawish
	 * kr vs km		-> look drawish
	 * knn vs k		-> very drawish
	 * kq vs kp
	 ----------------------------------------------*/


	Position p;
	U64 key;

	static const struct{
		std::string fen;
		materialStruct::tType type;
		bool (Position::*pointer)(Score &);
		Score val;
	} Endgames[] = {
			// DRAWN
			{"k7/8/8/8/8/8/8/7K w - -",materialStruct::exact, nullptr, 0 },

			{"kb6/8/8/8/8/8/8/7K w - -",materialStruct::exact, nullptr, 0 },
			{"k7/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },

			{"kn6/8/8/8/8/8/8/7K w - -",materialStruct::exact, nullptr, 0 },
			{"k7/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },

			{"kn6/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },
			{"kn6/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },

			{"knn5/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },
			{"knn5/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },
			{"kbn5/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },
			{"kbn5/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },
			{"kbb5/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },
			{"kbb5/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },

			{"kn6/8/8/8/8/8/8/5NNK w - -",materialStruct::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/5NNK w - -",materialStruct::exact, nullptr, 0 },
			{"kn6/8/8/8/8/8/8/5BNK w - -",materialStruct::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/5BNK w - -",materialStruct::exact, nullptr, 0 },
			{"kn6/8/8/8/8/8/8/5BBK w - -",materialStruct::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/5BBK w - -",materialStruct::exact, nullptr, 0 },

			{"kb6/8/8/8/8/8/7P/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/6PP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/5PPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/4PPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/3PPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/2PPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/1PPPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/PPPPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },

			{"k7/7p/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/6pp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/5ppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/4pppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/3ppppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/2pppppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/1ppppppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/pppppppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },

			{"kn6/8/8/8/8/8/7P/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/6PP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/5PPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/4PPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/3PPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/2PPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/1PPPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/PPPPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },

			{"k7/7p/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/6pp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/5ppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/4pppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/3ppppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/2pppppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/1ppppppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/pppppppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },

			{"k7/8/8/8/8/8/8/5BPK w - -",materialStruct::exactFunction, &Position::evalKBPvsK, 0 },
			{"kbp5/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKBPvsK, 0 },
			{"k7/8/8/8/8/8/8/5BNK w - -",materialStruct::exactFunction, &Position::evalKBNvsK, 0 },
			{"kbn5/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKBNvsK, 0 },

			{"k7/8/8/8/8/8/8/6QK w - -",materialStruct::exactFunction, &Position::evalKQvsK, 0 },
			{"kq6/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKQvsK, 0 },

			{"k7/8/8/8/8/8/8/6RK w - -",materialStruct::exactFunction, &Position::evalKRvsK, 0 },
			{"kr6/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKRvsK, 0 },

			{"k7/8/8/8/8/8/8/6PK w - -",materialStruct::exactFunction, &Position::evalKPvsK, 0 },
			{"kp6/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKPvsK, 0 },

			{"kr6/8/8/8/8/8/8/6NK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },
			{"kr6/8/8/8/8/8/8/6BK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },
			{"kb6/8/8/8/8/8/8/6RK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },
			{"kn6/8/8/8/8/8/8/6RK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },

			{"knn5/8/8/8/8/8/8/7K w - -",materialStruct::multiplicativeFunction, &Position::evalKNNvsK, 0 },
			{"k7/8/8/8/8/8/8/5NNK w - -",materialStruct::multiplicativeFunction, &Position::evalKNNvsK, 0 },

			{"kr6/8/8/8/8/8/8/5PRK w - -",materialStruct::multiplicativeFunction, &Position::evalKRPvsKr, 0 },
			{"krp5/8/8/8/8/8/8/6RK w - -",materialStruct::multiplicativeFunction, &Position::evalKRPvsKr, 0 },

			{"kq6/8/8/8/8/8/8/6PK w - -",materialStruct::exactFunction, &Position::evalKQvsKP, 0 },
			{"kp6/8/8/8/8/8/8/6QK w - -",materialStruct::exactFunction, &Position::evalKQvsKP, 0 }

	};

	materialStruct t;

	for (auto& eg : Endgames)
	{
		p.setupFromFen(eg.fen);
		key = p.getMaterialKey();
		t.type = eg.type;
		t.pointer = eg.pointer;
		t.val = eg.val;
		materialKeyMap.insert({key,t});
	}

	//------------------------------------------
	//	opposite bishop endgame
	//------------------------------------------
	for(int wp=0;wp<=8;wp++){
		for(int bp=0;bp<=8;bp++){
			if(wp!=0 || bp !=0){
				std::string s="kb6/";
				for(int i=0;i<bp;i++){
					s+="p";
				}
				if(bp!=8){s+=std::to_string(8-bp);}
				s+="/8/8/8/8/";
				for(int i=0;i<wp;i++){
					s+="P";
				}
				if(wp!=8){s+=std::to_string(8-wp);}
				s+="/6BK w - -";
				//sync_cout<<s<<sync_endl;
				p.setupFromFen(s);
				key = p.getMaterialKey();
				t.type=materialStruct::multiplicativeFunction;
				t.pointer=&Position::evalOppositeBishopEndgame;
				t.val=0;
				materialKeyMap.insert({key,t});
			}
		}
	}
}


//---------------------------------------------
const Position::materialStruct * Position::getMaterialData()
{
	U64 key = getMaterialKey();

	auto got= materialKeyMap.find(key);

	if( got == materialKeyMap.end())
	{
		return nullptr;
	}
	else
	{
		 return &(got->second);
	}

}




template<Color c>
simdScore Position::evalPawn(tSquare sq, bitMap& weakPawns, bitMap& passedPawns) const
{
	simdScore res = {0,0,0,0};

	bool passed, isolated, doubled, opposed, chain, backward;
	const bitMap ourPawns = c ? getBitmap(blackPawns) : getBitmap(whitePawns);
	const bitMap theirPawns = c ? getBitmap(whitePawns) : getBitmap(blackPawns);
	const int relativeRank = c ? 7 - bitHelper::getRank( sq ) : bitHelper::getRank( sq );

	// Our rank plus previous one. Used for chain detection
	bitMap b = bitHelper::getRankMask( sq ) | bitHelper::getRankMask( sq - pawnPush(c) );

	// Flag the pawn as passed, isolated, doubled or member of a pawn
    // chain (but not the backward one).
	bitMap neighborhood = bitHelper::getIsolatedBitmap(sq);
    chain    = (ourPawns	& neighborhood & b);
	isolated = !(ourPawns	& neighborhood);
    doubled  = (ourPawns	& bitHelper::getSquaresInFrontBitmap( c, sq ) );
    opposed  = (theirPawns	& bitHelper::getSquaresInFrontBitmap( c ,sq ) );
    passed   = !bitHelper::isBitmapPreventingPassedPawn( theirPawns, c, sq );

	backward = false;
	if(
		!(passed | isolated | chain) &&
		!bitHelper::isBitmapPreventingPassedPawn( ourPawns & neighborhood, (Color)(1 - c) , sq + pawnPush( c ) ) )// non ci sono nostri pedoni dietro a noi
	{
		b = bitHelper::getRankMask( sq + pawnPush( c ) ) & neighborhood;
		while ( !(b & (ourPawns | theirPawns)))
		{
			if(!c)
			{
				b <<= 8;
			}
			else
			{
				b >>= 8;
			}

		}
		backward = ((b | ( ( !c ) ? ( b << 8) : ( b >> 8 ) ) ) & theirPawns );
	}

	if (isolated)
	{
		if(opposed)
		{
			res -= isolatedPawnPenaltyOpp;
		}
		else
		{
			res -= isolatedPawnPenalty;
		}
		weakPawns += sq;
	}

    if( doubled )
    {
    	res -= doubledPawnPenalty;
	}

    if (backward)
    {
    	if(opposed)
    	{
			res -= backwardPawnPenalty / 2;
		}
		else
		{
			res -= backwardPawnPenalty;
		}
		weakPawns += sq;
	}

    if(chain)
    {
    	res += chainedPawnBonus;
	}
	else
	{
		weakPawns += sq;
	}


	//passed pawn
	if( passed && !doubled )
	{
		passedPawns += sq;
	}

	if ( !passed && !isolated && !doubled && !opposed && (relativeRank > 3) && bitCnt( bitHelper::getPassedPawnBitmap( c, sq ) & theirPawns ) < bitCnt( bitHelper::getPassedPawnBitmap( c, sq-pawnPush(c) ) & ourPawns ) )
	{
		res += candidateBonus * ( relativeRank - 1 );
	}
	return res;
}

template<Position::bitboardIndex piece>
simdScore Position::evalPieces(const bitMap * const weakSquares,  bitMap * const attackedSquares ,const bitMap * const holes,bitMap const blockedPawns, bitMap * const kingRing,unsigned int * const kingAttackersCount,unsigned int * const kingAttackersWeight,unsigned int * const kingAdjacentZoneAttacksCount, bitMap & weakPawns) const
{
	simdScore res = {0,0,0,0};
	bitMap tempPieces = getBitmap(piece);
	bitMap enemyKing = (piece > separationBitmap)? getBitmap(whiteKing) : getBitmap(blackKing);
	tSquare enemyKingSquare = (piece > separationBitmap)? getSquareOfThePiece(whiteKing) : getSquareOfThePiece(blackKing);
	bitMap enemyKingRing = (piece > separationBitmap) ? kingRing[white] : kingRing[black];
	unsigned int * pKingAttackersCount = (piece > separationBitmap) ? &kingAttackersCount[black] : &kingAttackersCount[white];
	unsigned int * pkingAttackersWeight = (piece > separationBitmap) ? &kingAttackersWeight[black] : &kingAttackersWeight[white];
	unsigned int * pkingAdjacentZoneAttacksCount = (piece > separationBitmap) ? &kingAdjacentZoneAttacksCount[black] : &kingAdjacentZoneAttacksCount[white];
	const bitMap & enemyWeakSquares = (piece > separationBitmap) ? weakSquares[white] : weakSquares[black];
	const bitMap & enemyHoles = (piece > separationBitmap) ? holes[white] : holes[black];
	const bitMap & supportedSquares = (piece > separationBitmap) ? attackedSquares[blackPawns] : attackedSquares[whitePawns];
	const bitMap & threatenSquares = (piece > separationBitmap) ? attackedSquares[whitePawns] : attackedSquares[blackPawns];
	const bitMap ourPieces = (piece > separationBitmap) ? getBitmap(blackPieces) : getBitmap(whitePieces);
	const bitMap ourPawns = (piece > separationBitmap) ? getBitmap(blackPawns) : getBitmap(whitePawns);
	const bitMap theirPieces = (piece > separationBitmap) ? getBitmap(whitePieces) : getBitmap(blackPieces);
	const bitMap theirPawns = (piece > separationBitmap) ? getBitmap(whitePawns) : getBitmap(blackPawns);

	(void)theirPawns;

	while(tempPieces)
	{
		tSquare sq = iterateBit(tempPieces);
		unsigned int relativeRank =(piece > separationBitmap) ? 7 - bitHelper::getRank( sq ) : bitHelper::getRank( sq );

		//---------------------------
		//	MOBILITY
		//---------------------------
		// todo mobility usare solo mosse valide ( pinned pieces)
		//todo mobility with pinned, mobility contando meno case attaccate da pezzi meno forti

		bitMap attack;
		switch(piece)
		{
			case Position::whiteRooks:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap() ^ getBitmap(whiteRooks) ^ getBitmap(whiteQueens));
				break;
			case Position::blackRooks:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap() ^ getBitmap(blackRooks) ^ getBitmap(blackQueens));
				break;
			case Position::whiteBishops:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap() ^ getBitmap(whiteQueens));
				break;
			case Position::blackBishops:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap() ^ getBitmap(blackQueens));
				break;
			case Position::whiteQueens:
			case Position::blackQueens:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap());
				break;
			case Position::whiteKnights:
			case Position::blackKnights:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap());
				break;
			default:
				break;
		}

		if(attack & enemyKingRing)
		{
			(*pKingAttackersCount)++;
			(*pkingAttackersWeight) += KingAttackWeights[ piece % separationBitmap];
			bitMap adjacent = attack & Movegen::attackFrom<whiteKing>(enemyKingSquare);
			if(adjacent)
			{
				( *pkingAdjacentZoneAttacksCount ) += bitCnt(adjacent);
			}
		}
		attackedSquares[piece] |= attack;


		bitMap defendedPieces = attack & ourPieces & ~ourPawns;
		// piece coordination
		res += bitCnt( defendedPieces ) * pieceCoordination;


		//unsigned int mobility = (bitCnt(attack&~(threatenSquares|ourPieces))+ bitCnt(attack&~(ourPieces)))/2;
		unsigned int mobility = bitCnt( attack & ~(threatenSquares | ourPieces));

		res += mobilityBonus[ piece % separationBitmap ][ mobility ];
		if(piece != whiteKnights && piece != blackKnights)
		{
			if( !(attack & ~(threatenSquares | ourPieces) )  && ( isSquareInBitmap( threatenSquares , sq) ) ) // zero mobility && attacked by pawn
			{
				res -= ( pieceValue[piece] / 4 );
			}
		}
		/////////////////////////////////////////
		// center control
		/////////////////////////////////////////
		if(attack & bitHelper::getCenterBitmap() )
		{
			res += bitCnt(attack & bitHelper::getCenterBitmap() ) * piecesCenterControl;
		}
		if(attack & bitHelper::getExtCenterBitmap())
		{
			res += bitCnt(attack & bitHelper::getExtCenterBitmap()) * piecesBigCenterControl;
		}

		switch(piece)
		{
		case Position::whiteQueens:
		case Position::blackQueens:
		{
			bitMap enemyBackRank = (piece > separationBitmap) ? bitHelper::getRankMask( A1 ) : bitHelper::getRankMask( A8 );
			bitMap enemyPawns = (piece > separationBitmap) ? getBitmap(whitePawns) : getBitmap(blackPawns);
			//--------------------------------
			// donna in 7a con re in 8a
			//--------------------------------
			if(relativeRank == 6 && (enemyKing & enemyBackRank) )
			{
				res += queenOn7Bonus;
			}
			//--------------------------------
			// donna su traversa che contiene pedoni
			//--------------------------------
			if(relativeRank > 4 && ( bitHelper::getRankMask( sq ) & enemyPawns))
			{
				res += queenOnPawns;
			}
			break;
		}
		case Position::whiteRooks:
		case Position::blackRooks:
		{
			bitMap enemyBackRank = (piece > separationBitmap) ? bitHelper::getRankMask( A1 ) : bitHelper::getRankMask( A8 );
			bitMap enemyPawns = (piece > separationBitmap) ?  getBitmap(whitePawns) : getBitmap(blackPawns);
			bitMap ourPawns = (piece > separationBitmap) ? getBitmap(blackPawns) : getBitmap(whitePawns);
			//--------------------------------
			// torre in 7a con re in 8a
			//--------------------------------
			if(relativeRank == 6 && (enemyKing & enemyBackRank) )
			{
				res += rookOn7Bonus;
			}
			//--------------------------------
			// torre su traversa che contiene pedoni
			//--------------------------------
			if(relativeRank > 4 && ( bitHelper::getRankMask( sq ) & enemyPawns))
			{
				res += rookOnPawns;
			}
			//--------------------------------
			// torre su colonna aperta/semiaperta
			//--------------------------------
			if( !(bitHelper::getFileMask( sq ) & ourPawns) )
			{
				if( !(bitHelper::getFileMask(sq) & enemyPawns) )
				{
					res += rookOnOpen;
				}else
				{
					res += rookOnSemi;
				}
			}
			//--------------------------------
			// torre intrappolata
			//--------------------------------
			else if( mobility < 3 )
			{

				tSquare ksq = (piece > separationBitmap) ?  getSquareOfThePiece(blackKing) : getSquareOfThePiece(whiteKing);
				unsigned int relativeRankKing = (piece > separationBitmap) ? 7 - bitHelper::getRank( ksq ) : bitHelper::getRank( ksq );

				if(
					( ( bitHelper::getFile( ksq ) < 4) == ( bitHelper::getFile( sq ) < bitHelper::getFile( ksq ) ) ) &&
					(bitHelper::getRank( ksq ) == bitHelper::getRank( sq ) && relativeRankKing == 0)
				)
				{

					res -= rookTrapped*(3-mobility);
					Position::state & st = getActualState();
					if(piece > separationBitmap)
					{
						if( !( st.castleRights & (Position::bCastleOO | Position::bCastleOOO) ) )
						{
							res -= rookTrappedKingWithoutCastling * ( 3 - mobility );
						}

					}
					else
					{
						if( !(st.castleRights & (Position::wCastleOO | Position::wCastleOOO) ) )
						{
							res -= rookTrappedKingWithoutCastling * ( 3 - mobility ) ;
						}
					}
				}
			}
			break;
		}
		case Position::whiteBishops:
		case Position::blackBishops:
			if(relativeRank >= 4 && ( isSquareInBitmap( enemyWeakSquares, sq) ))
			{
				res += bishopOnOutpost;
				if( isSquareInBitmap( supportedSquares, sq))
				{
					res += bishopOnOutpostSupported;
				}
				if(isSquareInBitmap(enemyHoles,sq))
				{
					res += bishopOnHole;
				}

			}
			// alfiere cattivo
			{
				Color color = bitHelper::getSquareColor( sq );
				bitMap blockingPawns = ourPieces & blockedPawns & bitHelper::getBitmapSquareColor( color ) ;
				if( moreThanOneBit(blockingPawns) )
				{
					res -= bitCnt(blockingPawns) * badBishop2;
				}
				blockingPawns &= ( bitHelper::getBigCenterBitmap() );
				if( moreThanOneBit(blockingPawns) )
				{
					res -= bitCnt(blockingPawns) * badBishop1;
				}
			}

			break;
		case Position::whiteKnights:
		case Position::blackKnights:
			if( isSquareInBitmap( enemyWeakSquares, sq))
			{
				res += knightOnOutpost * ( 5 - std::abs( (int)relativeRank - 5 ));
				if( isSquareInBitmap( supportedSquares, sq))
				{
					res += knightOnOutpostSupported;
				}
				if( isSquareInBitmap( enemyHoles,sq))
				{
					res += knightOnHole;
				}

			}

			{
				bitMap wpa = attack & (weakPawns) & theirPieces;
				if(wpa)
				{
					res += bitCnt(wpa) * KnightAttackingWeakPawn;
				}
			}
			break;
		default:
			break;
		}
	}
	return res;
}

template<Color kingColor>
Score Position::evalShieldStorm(tSquare ksq) const
{
	Score ks = 0;
	const bitMap ourPawns = kingColor ? getBitmap(blackPawns) : getBitmap(whitePawns);
	const bitMap theirPawns = kingColor ? getBitmap(whitePawns) : getBitmap(blackPawns);
	const unsigned int disableRank= kingColor ? 0: 7;
	bitMap localKingRing = Movegen::attackFrom<Position::whiteKing>(ksq);
	bitMap localKingShield = localKingRing;

	if( bitHelper::getRank( ksq ) != disableRank )
	{
		localKingRing |= Movegen::attackFrom<Position::whiteKing>(ksq + pawnPush(kingColor));
	}
	bitMap localKingFarShield = localKingRing & ~(localKingShield);

	bitMap pawnShield = localKingShield & ourPawns;
	bitMap pawnFarShield = localKingFarShield & ourPawns;
	bitMap pawnStorm = bitHelper::isBitmapPreventingPassedPawn( theirPawns, kingColor, ksq );
	if(pawnShield)
	{
		ks = bitCnt(pawnShield) * kingShieldBonus;
	}
	if(pawnFarShield)
	{
		ks += bitCnt(pawnFarShield) * kingFarShieldBonus;
	}
	while(pawnStorm)
	{
		tSquare p = iterateBit(pawnStorm);
		ks -= ( 7 - bitHelper::getSquareDistance( p, ksq ) ) * kingStormBonus;
	}
	return ks;
}

template<Color c>
simdScore Position::evalPassedPawn(bitMap pp, bitMap* attackedSquares) const
{
	tSquare kingSquare = c ? getSquareOfThePiece( blackKing ) : getSquareOfThePiece( whiteKing );
	tSquare enemyKingSquare = c ?getSquareOfThePiece( whiteKing ) : getSquareOfThePiece( blackKing );
	bitboardIndex ourPieces = c ? blackPieces : whitePieces;
	bitboardIndex enemyPieces = c ? whitePieces : blackPieces;
	bitboardIndex ourRooks = c ? blackRooks : whiteRooks;
	bitboardIndex enemyRooks = c ? whiteRooks : blackRooks;
	bitboardIndex ourPawns = c ? blackPawns : whitePawns;
	state st = getActualState();

	simdScore score = {0,0,0,0};
	while(pp)
	{
		simdScore passedPawnsBonus;
		tSquare ppSq = iterateBit(pp);

		unsigned int relativeRank = c ? 7 - bitHelper::getRank( ppSq ) : bitHelper::getRank( ppSq );

		int r = relativeRank - 1;
		int rr =  r * ( r - 1 );

		passedPawnsBonus = simdScore{ passedPawnBonus[0] * rr, passedPawnBonus[1] * ( rr + r + 1 ), 0, 0};

		if(rr)
		{
			tSquare blockingSquare = ppSq + pawnPush(c);

			// bonus for king proximity to blocking square
			passedPawnsBonus += enemyKingNearPassedPawn * ( bitHelper::getSquareDistance( blockingSquare,  enemyKingSquare ) * rr );
			passedPawnsBonus -= ownKingNearPassedPawn * ( bitHelper::getSquareDistance( blockingSquare, kingSquare ) * rr );

			if( getPieceAt(blockingSquare) == empty )
			{
				bitMap forwardSquares = bitHelper::getSquaresInFrontBitmap( c, ppSq );
				bitMap backWardSquares = bitHelper::getSquaresInFrontBitmap( (Color)(1 - c), ppSq );

				bitMap defendedSquares = forwardSquares & attackedSquares[ ourPieces ];

				bitMap unsafeSquares = forwardSquares & (attackedSquares[enemyPieces] | getBitmap(enemyPieces) );

				if(unsafeSquares)
				{
					passedPawnsBonus -= passedPawnUnsafeSquares * rr;
					if ( isSquareInBitmap( unsafeSquares, blockingSquare) )
					{
						passedPawnsBonus -= passedPawnBlockedSquares * rr;
					}
				}
				if(defendedSquares)
				{
					passedPawnsBonus += passedPawnDefendedSquares * rr * bitCnt( defendedSquares );
					if( isSquareInBitmap( defendedSquares, blockingSquare) )
					{
						passedPawnsBonus += passedPawnDefendedBlockingSquare * rr;
					}
				}
				if( backWardSquares & getBitmap( ourRooks ))
				{
					passedPawnsBonus += rookBehindPassedPawn * rr;
				}
				if(backWardSquares & getBitmap( enemyRooks ))
				{
					passedPawnsBonus -= EnemyRookBehindPassedPawn * rr;
				}
			}
		}

		if( bitHelper::getFile( ppSq ) == 0 || bitHelper::getFile( ppSq ) == 7)
		{
			passedPawnsBonus -= passedPawnFileAHPenalty;
		}

		bitMap supportingPawns = getBitmap( ourPawns ) & bitHelper::getIsolatedBitmap( ppSq );
		if( supportingPawns & bitHelper::getRankMask( ppSq ) )
		{
			passedPawnsBonus+=passedPawnSupportedBonus*r;
		}
		if( supportingPawns & bitHelper::getRankMask( ppSq - pawnPush(c) ) )
		{
			passedPawnsBonus += passedPawnSupportedBonus * ( r / 2 );
		}

		if( st.nonPawnMaterial[ c ? 0 : 2 ] == 0 )
		{
			tSquare promotionSquare = bitHelper::getTsquareFromFileRank( bitHelper::getFile( ppSq ), c ? 0 : 7 );
			if ( std::min( 5, (int)(7- relativeRank)) <  std::max( (int)bitHelper::getSquareDistance( enemyKingSquare, promotionSquare ) - (st.nextMove == (c ? blackTurn : whiteTurn) ? 0 : 1 ), 0) )
			{
				passedPawnsBonus += unstoppablePassed * rr;
			}
		}


		score += passedPawnsBonus;

	}
	return score;
}


template<Color c> simdScore Position::evalKingSafety(Score kingSafety, unsigned int kingAttackersCount, unsigned int kingAdjacentZoneAttacksCount, unsigned int kingAttackersWeight, bitMap * const attackedSquares) const
{
	simdScore res = {0,0,0,0};
	//bitMap * OurPieces = c ? &getBitmap(separationBitmap) : &getBitmap(occupiedSquares);
	bitMap TheirPieces = c ? getBitmap(whitePieces) : getBitmap(blackPieces);
	bitMap * AttackedByOurPieces = c ? &attackedSquares[separationBitmap] : &attackedSquares[occupiedSquares];
	bitMap * AttackedByTheirPieces = c ? &attackedSquares[occupiedSquares] : &attackedSquares[separationBitmap];
	tSquare kingSquare = c ? getSquareOfThePiece(blackKing) : getSquareOfThePiece(whiteKing);

	if( kingAttackersCount )// se il re e' attaccato
	{

		bitMap undefendedSquares = AttackedByTheirPieces[Pieces] & AttackedByOurPieces[King];
		undefendedSquares &=
			~(AttackedByOurPieces[Pawns]
			| AttackedByOurPieces[Knights]
			| AttackedByOurPieces[Bishops]
			| AttackedByOurPieces[Rooks]
			| AttackedByOurPieces[Queens]);


		signed int attackUnits =  std::min((unsigned int)25, (kingAttackersCount * kingAttackersWeight) / 2)
							 + 3 * (kingAdjacentZoneAttacksCount + bitCnt( undefendedSquares ) )
							 + KingExposed[c? 63 - kingSquare : kingSquare ]
							 - kingSafety / kingSafetyScaling[0]
							 - (getPieceCount(c? whiteQueens: blackQueens)==0)*40;

		// safe contact queen check
		bitMap safeContactSquare = undefendedSquares & AttackedByTheirPieces[Queens]  & ~TheirPieces;
		safeContactSquare &= (AttackedByTheirPieces[Rooks]| AttackedByTheirPieces[Bishops] | AttackedByTheirPieces[Knights]| AttackedByTheirPieces[Pawns]);
		if(safeContactSquare)
		{
			attackUnits += 20 * bitCnt(safeContactSquare);
		}

		// safe contact rook check
		safeContactSquare = undefendedSquares & AttackedByTheirPieces[Rooks] & ~TheirPieces;
		safeContactSquare &= (AttackedByTheirPieces[Queens]| AttackedByTheirPieces[Bishops] |AttackedByTheirPieces[Knights]| AttackedByTheirPieces[Pawns]);

		safeContactSquare &= Movegen::getRookPseudoAttack( kingSquare );

		if(safeContactSquare)
		{
			attackUnits += 15 * bitCnt(safeContactSquare);
		}


		// long distance check
		bitMap rMap = Movegen::attackFrom<whiteRooks>( kingSquare, getOccupationBitmap() );
		bitMap bMap = Movegen::attackFrom<whiteBishops>( kingSquare, getOccupationBitmap() );

		// vertical check
		bitMap longDistCheck = rMap & (AttackedByTheirPieces[Rooks] | AttackedByTheirPieces[Queens] ) & ~AttackedByOurPieces[Pieces] & ~TheirPieces;
		if(longDistCheck)
		{
			attackUnits += 8 * bitCnt( longDistCheck );
		}

		// diagonal check
		longDistCheck = bMap & (AttackedByTheirPieces[Bishops] | AttackedByTheirPieces[Queens] ) & ~AttackedByOurPieces[Pieces] & ~TheirPieces;
		if(longDistCheck)
		{
			attackUnits += 3 * bitCnt( longDistCheck );
		}

		///knight check;
		longDistCheck = Movegen::attackFrom<whiteKnights>( kingSquare ) & ( AttackedByTheirPieces[Knights] ) & ~AttackedByOurPieces[Pieces] & ~TheirPieces;
		if(longDistCheck)
		{
			attackUnits += bitCnt( longDistCheck );
		}

		attackUnits = std::min(KingSafetyMaxAttack[0], std::max(0, attackUnits));
		attackUnits *= std::min(KingSafetyLinearCoefficent[0], attackUnits);
		attackUnits = std::min(KingSafetyMaxResult[0], attackUnits);
		res = -(kingSafetyBonus * attackUnits);

	}
	return res;
}

/*! \brief do a pretty simple evalutation
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
template<bool trace>
Score Position::eval(void)
{

	const state &st = getActualState();

	if(trace)
	{

		sync_cout << std::setprecision(3) << std::setw(21) << "Eval term " << "|     White    |     Black    |      Total     \n"
			      <<             "                     |    MG    EG  |   MG     EG  |   MG      EG   \n"
			      <<             "---------------------+--------------+--------------+-----------------" << sync_endl;
	}

	Score lowSat = -SCORE_INFINITE;
	Score highSat = SCORE_INFINITE;
	Score mulCoeff = 256;

	bitMap attackedSquares[lastBitboard] = { 0 };
	bitMap weakSquares[2];
	bitMap holes[2];
	bitMap kingRing[2];
	bitMap kingShield[2];
	bitMap kingFarShield[2];
	unsigned int kingAttackersCount[2] = {0};
	unsigned int kingAttackersWeight[2] = {0};
	unsigned int kingAdjacentZoneAttacksCount[2] = {0};

	tSquare k = getSquareOfThePiece(whiteKing);
	kingRing[white] = Movegen::attackFrom<Position::whiteKing>(k);
	kingShield[white] = kingRing[white];
	if( bitHelper::getRank( k ) < 7 )
	{
		kingRing[white] |= Movegen::attackFrom<Position::whiteKing>( tSquare( k + 8) );
	}
	kingFarShield[white] = kingRing[white] & ~( kingShield[white] | bitHelper::getBitmapFromSquare(k) );


	k = getSquareOfThePiece(blackKing);
	kingRing[black] = Movegen::attackFrom<Position::whiteKing>(k);
	kingShield[black] = kingRing[black];
	if( bitHelper::getRank( k ) > 0 )
	{
		kingRing[black] |= Movegen::attackFrom<Position::whiteKing>( tSquare( k - 8 ) );
	}
	kingFarShield[black] = kingRing[black] & ~( kingShield[black] | bitHelper::getBitmapFromSquare(k) );

	// todo modificare valori material value & pst
	// material + pst

	simdScore res = st.material;


	//-----------------------------------------------------
	//	material evalutation
	//-----------------------------------------------------
	const materialStruct* materialData = getMaterialData();
	if( materialData )
	{
		bool (Position::*pointer)(Score &) = materialData->pointer;
		switch(materialData->type)
		{
			case materialStruct::exact:
				return st.nextMove? -materialData->val : materialData->val;
				break;
			case materialStruct::multiplicativeFunction:
			{
				Score r;
				if( (this->*pointer)(r))
				{
					mulCoeff = r;
				}
				break;
			}
			case materialStruct::exactFunction:
			{
				Score r = 0;
				if( (this->*pointer)(r))
				{
					return st.nextMove? -r : r;
				}
				break;
			}
			case materialStruct::saturationH:
				highSat = materialData->val;
				break;
			case materialStruct::saturationL:
				lowSat = materialData->val;
				break;
		}
	}

	//---------------------------------------------
	//	tempo
	//---------------------------------------------
	res += st.nextMove ? -tempo : tempo;

	if(trace)
	{
		sync_cout << std::setw(20) << "Material, PST, Tempo" << " |   ---    --- |   ---    --- | "
		          << std::setw(6)  << res[0]/10000.0 << " "
		          << std::setw(6)  << res[1]/10000.0 << " "<< sync_endl;
	}


	//---------------------------------------------
	//	imbalancies
	//---------------------------------------------
	//	bishop pair

	simdScore imbalances= {0};
	if( (getBitmap(whiteBishops) & bitHelper::getBitmapSquareColor( Color::white ) ) && (getBitmap(whiteBishops) & bitHelper::getBitmapSquareColor( Color::black ) ) )
	{
		imbalances += bishopPair;
	}

	if( (getBitmap(blackBishops) & bitHelper::getBitmapSquareColor( Color::white ) ) && (getBitmap(blackBishops) & bitHelper::getBitmapSquareColor( Color::black ) ) )
	{
		imbalances -= bishopPair;
	}
	if( getPieceCount(blackPawns) + getPieceCount(whitePawns) == 0 )
	{
		if((int)getPieceCount(whiteQueens) - (int)getPieceCount(blackQueens) == 1
				&& (int)getPieceCount(blackRooks) - (int)getPieceCount(whiteRooks) == 1
				&& (int)getPieceCount(blackBishops) + (int)getPieceCount(blackKnights) - (int)getPieceCount(whiteBishops) - (int)getPieceCount(whiteKnights) == 2)
		{
			imbalances += queenVsRook2MinorsImbalance;

		}
		else if((int)getPieceCount(whiteQueens) - (int)getPieceCount(blackQueens) == -1
				&& (int)getPieceCount(blackRooks) - (int)getPieceCount(whiteRooks) == -1
				&& (int)getPieceCount(blackBishops) + (int)getPieceCount(blackKnights) - (int)getPieceCount(whiteBishops) -(int)getPieceCount(whiteKnights) == -2)
		{
			imbalances -= queenVsRook2MinorsImbalance;

		}
	}

	res += imbalances;

	if(trace)
	{
		sync_cout << std::setw(20) << "imbalancies" << " |   ---    --- |   ---    --- | "
				  << std::setw(6)  << (imbalances[0])/10000.0 << " "
				  << std::setw(6)  << (imbalances[1])/10000.0 << " " << sync_endl;
	}


	//todo specialized endgame & scaling function
	//todo material imbalance
	bitMap weakPawns = 0;
	bitMap passedPawns = 0;




	//----------------------------------------------
	//	PAWNS EVALUTATION
	//----------------------------------------------
	simdScore pawnResult;
	U64 pawnKey = getPawnKey();
	pawnEntry& probePawn = pawnHashTable.probe(pawnKey);
	if( probePawn.key == pawnKey)
	{
		pawnResult = simdScore{probePawn.res[0], probePawn.res[1], 0, 0};
		weakPawns = probePawn.weakPawns;
		passedPawns = probePawn.passedPawns;
		attackedSquares[whitePawns] = probePawn.pawnAttacks[0];
		attackedSquares[blackPawns] = probePawn.pawnAttacks[1];
		weakSquares[white] = probePawn.weakSquares[0];
		weakSquares[black] = probePawn.weakSquares[1];
		holes[white] = probePawn.holes[0];
		holes[black] = probePawn.holes[1];
	}
	else
	{


		pawnResult = simdScore{0,0,0,0};
		bitMap pawns = getBitmap(whitePawns);

		while(pawns)
		{
			tSquare sq = iterateBit(pawns);
			pawnResult += evalPawn<white>(sq, weakPawns, passedPawns);
		}

		pawns = getBitmap(blackPawns);

		while(pawns)
		{
			tSquare sq = iterateBit(pawns);
			pawnResult -= evalPawn<black>(sq, weakPawns, passedPawns);
		}



		bitMap temp = getBitmap(whitePawns);
		bitMap pawnAttack = ( temp & ~( bitHelper::getFileMask( H1 ) ) ) << 9;
		pawnAttack |= (temp & ~( bitHelper::getFileMask( A1 ) ) ) << 7;

		attackedSquares[whitePawns] = pawnAttack;
		pawnAttack |= pawnAttack << 8;
		pawnAttack |= pawnAttack << 16;
		pawnAttack |= pawnAttack << 32;

		weakSquares[white] = ~pawnAttack;


		temp = getBitmap(blackPawns);
		pawnAttack = ( temp & ~( bitHelper::getFileMask(H1 ) ) ) >> 7;
		pawnAttack |= ( temp & ~( bitHelper::getFileMask(A1) ) ) >> 9;

		attackedSquares[blackPawns] = pawnAttack;

		pawnAttack |= pawnAttack >> 8;
		pawnAttack |= pawnAttack >> 16;
		pawnAttack |= pawnAttack >> 32;

		weakSquares[black] = ~pawnAttack;

		temp = getBitmap(whitePawns) << 8;
		temp |= temp << 8;
		temp |= temp << 16;
		temp |= temp << 32;

		holes[white] = weakSquares[white] & temp;



		temp = getBitmap(blackPawns) >> 8;
		temp |= temp >> 8;
		temp |= temp >> 16;
		temp |= temp >> 32;

		holes[black] = weakSquares[black] & temp;
		pawnResult -= ( bitCnt( holes[white] ) - bitCnt( holes[black] ) ) * holesPenalty;

		pawnHashTable.insert(pawnKey, pawnResult, weakPawns, passedPawns, attackedSquares[whitePawns], attackedSquares[blackPawns], weakSquares[white], weakSquares[black], holes[white], holes[black] );

	}
	//---------------------------------------------
	// center control
	//---------------------------------------------

	if( attackedSquares[whitePawns] & bitHelper::getCenterBitmap() )
	{
		pawnResult += bitCnt( attackedSquares[whitePawns] & bitHelper::getCenterBitmap() ) * pawnCenterControl;
	}
	if( attackedSquares[whitePawns] & bitHelper::getExtCenterBitmap() )
	{
		pawnResult += bitCnt( attackedSquares[whitePawns] & bitHelper::getExtCenterBitmap() )* pawnBigCenterControl;
	}

	if( attackedSquares[blackPawns] & bitHelper::getCenterBitmap() )
	{
		pawnResult -= bitCnt( attackedSquares[blackPawns] & bitHelper::getCenterBitmap() ) * pawnCenterControl;
	}
	if( attackedSquares[blackPawns] & bitHelper::getExtCenterBitmap() )
	{
		pawnResult -= bitCnt( attackedSquares[blackPawns] & bitHelper::getExtCenterBitmap() ) * pawnBigCenterControl;
	}

	res += pawnResult;

	if(trace)
	{
		sync_cout << std::setw(20) << "pawns" << " |   ---    --- |   ---    --- | "
				  << std::setw(6)  << (pawnResult[0])/10000.0 << " "
				  << std::setw(6)  << (pawnResult[1])/10000.0 << " " << sync_endl;
	}


	//-----------------------------------------
	//	blocked pawns
	//-----------------------------------------
	bitMap blockedPawns = ( getBitmap(whitePawns)<<8 ) & getBitmap( blackPawns );
	blockedPawns |= blockedPawns >> 8;



	//-----------------------------------------
	//	pieces
	//-----------------------------------------

	simdScore wScore;
	simdScore bScore;
	simdScore PieceScore;
	wScore = evalPieces<Position::whiteKnights>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = evalPieces<Position::blackKnights>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	PieceScore = wScore - bScore;
	res += PieceScore;
	if(trace)
	{
		sync_cout << std::setw(20) << "knights" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (PieceScore[0])/10000.0 << " "
				  << std::setw(6)  << (PieceScore[1])/10000.0 << " " << sync_endl;
	}

	wScore = evalPieces<Position::whiteBishops>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = evalPieces<Position::blackBishops>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	PieceScore = wScore - bScore;
	res += PieceScore;
	if(trace)
	{
		sync_cout << std::setw(20) << "bishops" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (PieceScore[0])/10000.0 << " "
				  << std::setw(6)  << (PieceScore[1])/10000.0 << " " << sync_endl;
	}

	wScore = evalPieces<Position::whiteRooks>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = evalPieces<Position::blackRooks>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	PieceScore = wScore - bScore;
	res += PieceScore;
	if(trace)
	{
		sync_cout << std::setw(20) << "rooks" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (PieceScore[0])/10000.0 << " "
				  << std::setw(6)  << (PieceScore[1])/10000.0 << " " << sync_endl;
	}

	wScore = evalPieces<Position::whiteQueens>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = evalPieces<Position::blackQueens>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	PieceScore = wScore - bScore;
	res += PieceScore;

	if(trace)
	{
		sync_cout << std::setw(20) << "queens" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (PieceScore[0])/10000.0 << " "
				  << std::setw(6)  << (PieceScore[1])/10000.0 << " " << sync_endl;
	}


	attackedSquares[whiteKing] = Movegen::attackFrom<Position::whiteKing>(getSquareOfThePiece(whiteKing));
	attackedSquares[blackKing] = Movegen::attackFrom<Position::blackKing>(getSquareOfThePiece(blackKing));

	attackedSquares[whitePieces] = attackedSquares[whiteKing]
								| attackedSquares[whiteKnights]
								| attackedSquares[whiteBishops]
								| attackedSquares[whiteRooks]
								| attackedSquares[whiteQueens]
								| attackedSquares[whitePawns];

	attackedSquares[blackPieces] = attackedSquares[blackKing]
								| attackedSquares[blackKnights]
								| attackedSquares[blackBishops]
								| attackedSquares[blackRooks]
								| attackedSquares[blackQueens]
								| attackedSquares[blackPawns];


	//-----------------------------------------
	//	passed pawn evalutation
	//-----------------------------------------


	wScore = evalPassedPawn<white>(passedPawns & getBitmap( whitePawns ), attackedSquares);
	bScore = evalPassedPawn<black>(passedPawns & getBitmap( blackPawns ), attackedSquares);
	PieceScore = wScore - bScore;
	res += PieceScore;

	if(trace)
	{
		sync_cout << std::setw(20) << "passed pawns" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (PieceScore[0])/10000.0 << " "
				  << std::setw(6)  << (PieceScore[1])/10000.0 << " " << sync_endl;
	}
	//todo attacked squares

	//---------------------------------------
	//	space
	//---------------------------------------
	// white pawns
	bitMap spacew = getBitmap(whitePawns);
	spacew |= spacew >> 8;
	spacew |= spacew >> 16;
	spacew |= spacew >> 32;
	spacew &= ~attackedSquares[blackPieces];

	// black pawns
	bitMap spaceb = getBitmap(blackPawns);
	spaceb |= spaceb << 8;
	spaceb |= spaceb << 16;
	spaceb |= spaceb << 32;
	spaceb &= ~attackedSquares[whitePieces];

	PieceScore = ( bitCnt(spacew) - bitCnt(spaceb) ) * spaceBonus;
	res += PieceScore;

	if(trace)
	{
		wScore = bitCnt(spacew) * spaceBonus;
		bScore = bitCnt(spaceb) * spaceBonus;
		sync_cout << std::setw(20) << "space" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (PieceScore[0])/10000.0 << " "
				  << std::setw(6)  << (PieceScore[1])/10000.0 << " " << sync_endl;
	}


	//todo counterattack??

	//todo weakpawn

	//--------------------------------------
	//	weak pieces
	//--------------------------------------
	wScore = simdScore{0,0,0,0};
	bScore = simdScore{0,0,0,0};

	bitMap pawnAttackedPieces = getBitmap( whitePieces ) & attackedSquares[ blackPawns ];
	while(pawnAttackedPieces)
	{
		tSquare attacked = iterateBit( pawnAttackedPieces );
		wScore -= attackedByPawnPenalty[ getPieceAt(attacked) % separationBitmap ];
	}

	// todo fare un weak piece migliore:qualsiasi pezzo attaccato riceve un malus dipendente dal suo più debole attaccante e dal suo valore.
	// volendo anche da quale pezzo è difeso
	bitMap undefendedMinors =  (getBitmap(whiteKnights) | getBitmap(whiteBishops))  & ~attackedSquares[whitePieces];
	if (undefendedMinors)
	{
		wScore -= undefendedMinorPenalty;
	}
	bitMap weakPieces = getBitmap(whitePieces) & attackedSquares[blackPieces] & ~attackedSquares[whitePawns];
	while(weakPieces)
	{
		tSquare p = iterateBit(weakPieces);

		bitboardIndex attackedPiece = getPieceAt(p);
		bitboardIndex attackingPiece = blackPawns;
		for(; attackingPiece >= blackKing; attackingPiece = (bitboardIndex)(attackingPiece - 1) )
		{
			if( isSquareInBitmap( attackedSquares[ attackingPiece ], p))
			{
				wScore -= weakPiecePenalty[attackedPiece % separationBitmap][ attackingPiece % separationBitmap];
				break;
			}
		}

	}

	pawnAttackedPieces = getBitmap( blackPieces ) & attackedSquares[ whitePawns ];
	while(pawnAttackedPieces)
	{
		tSquare attacked = iterateBit( pawnAttackedPieces );
		bScore -= attackedByPawnPenalty[ getPieceAt(attacked) % separationBitmap ];
	}

	undefendedMinors =  (getBitmap(blackKnights) | getBitmap(blackBishops))  & ~attackedSquares[blackPieces];
	if (undefendedMinors)
	{
		bScore -= undefendedMinorPenalty;
	}
	weakPieces = getBitmap(blackPieces) & attackedSquares[whitePieces] & ~attackedSquares[blackPawns];
	while(weakPieces)
	{
		tSquare p = iterateBit(weakPieces);

		bitboardIndex attackedPiece = getPieceAt(p);
		bitboardIndex attackingPiece = whitePawns;
		for(; attackingPiece >= whiteKing; attackingPiece = (bitboardIndex)(attackingPiece - 1) )
		{
			if( isSquareInBitmap( attackedSquares[ attackingPiece ] ,p ))
			{
				bScore -= weakPiecePenalty[attackedPiece % separationBitmap][attackingPiece % separationBitmap];
				break;
			}
		}
	}

	// trapped pieces
	// rook trapped on first rank by own king

	PieceScore = wScore - bScore;
	res += PieceScore;

	if(trace)
	{
		sync_cout << std::setw(20) << "threat" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (PieceScore[0])/10000.0 << " "
				  << std::setw(6)  << (PieceScore[1])/10000.0 << " " << sync_endl;
	}

	//--------------------------------------
	//	king safety
	//--------------------------------------
	Score kingSafety[2] = {0, 0};

	kingSafety[white] = evalShieldStorm<white>(getSquareOfThePiece(whiteKing));

	if((st.castleRights & wCastleOO)
		&& !(attackedSquares[blackPieces] & (bitHelper::getBitmapFromSquare(E1) | bitHelper::getBitmapFromSquare(F1) | bitHelper::getBitmapFromSquare(G1) ))
		&& bitCnt(getOccupationBitmap() &  Movegen::getCastlePath( Color::white, Movegen::kingSideCastle) ) <= 1
		)
	{
		kingSafety[white] = std::max( evalShieldStorm<white>(G1), kingSafety[white]);
	}

	if((st.castleRights & wCastleOOO)
		&& !(attackedSquares[blackPieces] & (bitHelper::getBitmapFromSquare(E1) | bitHelper::getBitmapFromSquare(D1) | bitHelper::getBitmapFromSquare(C1) ))
		&& bitCnt(getOccupationBitmap() & Movegen::getCastlePath( Color::white, Movegen::queenSideCastle) ) <=1
		)
	{
		kingSafety[white] = std::max( evalShieldStorm<white>(C1), kingSafety[white]);
	}
	if(trace)
	{
		wScore = simdScore{ kingSafety[white], 0, 0, 0};
	}

	kingSafety[black] = evalShieldStorm<black>(getSquareOfThePiece(blackKing));

	if((st.castleRights & bCastleOO)
		&& !(attackedSquares[whitePieces] & (bitHelper::getBitmapFromSquare(E8) | bitHelper::getBitmapFromSquare(F8) | bitHelper::getBitmapFromSquare(G8) ))
		&& bitCnt(getOccupationBitmap() & Movegen::getCastlePath( Color::black, Movegen::kingSideCastle) ) <=1
		)
	{
		kingSafety[black] = std::max( evalShieldStorm<black>(G8), kingSafety[black]);
	}

	if((st.castleRights & bCastleOOO)
		&& !(attackedSquares[whitePieces] & (bitHelper::getBitmapFromSquare(E8) | bitHelper::getBitmapFromSquare(D8) | bitHelper::getBitmapFromSquare(C8) ))
		&& bitCnt(getOccupationBitmap() & Movegen::getCastlePath(Color::black, Movegen::queenSideCastle)) <=1
		)
	{
		kingSafety[black] = std::max(evalShieldStorm<black>(C8), kingSafety[black]);
	}
	if(trace)
	{
		bScore = simdScore{ kingSafety[black], 0, 0, 0};
	}

	PieceScore = simdScore{kingSafety[white]-kingSafety[black],0,0,0};




	simdScore kingSaf = evalKingSafety<white>(kingSafety[white], kingAttackersCount[black], kingAdjacentZoneAttacksCount[black], kingAttackersWeight[black], attackedSquares);

	PieceScore += kingSaf;
	if(trace)
	{
		wScore += kingSaf;
	}

	kingSaf = evalKingSafety<black>(kingSafety[black], kingAttackersCount[white], kingAdjacentZoneAttacksCount[white], kingAttackersWeight[white], attackedSquares);

	PieceScore-= kingSaf;
	if(trace)
	{
		bScore += kingSaf;
	}

	res += PieceScore;

	if(trace)
	{
		sync_cout << std::setw(20) << "king safety" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (PieceScore[0])/10000.0 << " "
				  << std::setw(6)  << (PieceScore[1])/10000.0 << " "<< sync_endl;
		sync_cout << "---------------------+--------------+--------------+-----------------" << sync_endl;
		sync_cout << std::setw(20) << "result" << " |"
				  << std::setw(6)  << " " << " "
				  << std::setw(6)  << " "<< " |"
				  << std::setw(6)  << " "<< " "
				  << std::setw(6)  << " " << " | "
				  << std::setw(6)  << (res[0])/10000.0 << " "
				  << std::setw(6)  << (res[1])/10000.0 << " " << sync_endl;
	}

	//todo scaling
	if(mulCoeff == 256 && (getPieceCount(whitePawns) + getPieceCount(blackPawns) == 0 ) && (abs( st.material[0] )< 40000) )
	{
		//Score sumMaterial = st.nonPawnMaterial[0] + st.nonPawnMaterial[2];
		//mulCoeff = std::max(std::min((Score) (sumMaterial* 0.0003 - 14), 256), 40);
		if( (st.nonPawnMaterial[0]< 90000) && (st.nonPawnMaterial[2] < 90000) )
		{
			mulCoeff = 40;
		}
		//else
		//{
		//	mulCoeff = 65;
		//}

	}

	if(mulCoeff == 256  && st.nonPawnMaterial[0] + st.nonPawnMaterial[2] < 40000  &&  (st.nonPawnMaterial[0] + st.nonPawnMaterial[2] !=0) && (getPieceCount(whitePawns) == getPieceCount(blackPawns)) && !passedPawns )
	{
		mulCoeff = std::min((unsigned int)256, getPieceCount(whitePawns) * 80);
	}






	//--------------------------------------
	//	finalizing
	//--------------------------------------
	signed int gamePhase = getGamePhase();
	signed long long r = ( (signed long long)res[0] ) * ( 65536 - gamePhase ) + ( (signed long long)res[1] ) * gamePhase;

	Score score = (Score)( (r) / 65536 );
	if(mulCoeff != 256)
	{
		score *= mulCoeff;
		score /= 256;
	}

	// final value saturation
	score = std::min(highSat,score);
	score = std::max(lowSat,score);

	return st.nextMove ? -score : score;

}

template Score Position::eval<false>(void);
template Score Position::eval<true>(void);
