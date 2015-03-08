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

#include "vajolet.h"

#include "io.h"
#include "move.h"
#include "position.h"
#include "movegen.h"
#include "bitops.h"
Score seeScore[Position::lastBitboard]={
		0,
		3000000,//king
		137000,//queen
		52000,//rook
		35300,//bishop
		34500,//knight
		5700,//panws
		0,
		0,
		3000000,
		137000,
		52000,
		35300,
		34500,
		5700,
		0
};

Score Position::seeSign(Move m) const {
	assert(m.packed);
	if (seeScore[squares[m.from]] <= seeScore[squares[m.to]])
	{
		return 1;
	}

	return see(m);
}





Score Position::see(Move m) const {

	assert(m.packed);
#ifdef DEBUG_SEE
	sync_cout<<"--------------------------------------------"<<sync_endl;
	display();
	sync_cout<<displayUci(m)<<sync_endl;
#endif


	tSquare from=(tSquare)m.from, to=(tSquare)m.to;
	const int relativeRank =getActualState().nextMove?7-RANKS[to] :RANKS[to];
	bitMap occupied=bitBoard[occupiedSquares]^bitSet(from);
//	displayBitMap(occupied);
	eNextMove color=squares[from]>separationBitmap?blackTurn:whiteTurn;

	Score swapList[64];
	unsigned int slIndex = 1;
	bitMap colorAttackers;
	bitboardIndex captured;

	swapList[0] = seeScore[squares[to]];
	captured = bitboardIndex(squares[from]%separationBitmap);

	if(m.flags== Move::fenpassant){
		occupied ^= to- pawnPush(color);
		swapList[0] = seeScore[whitePawns];
	}
	if(m.flags== Move::fcastle){
		return 0;
	}
	if(m.flags== Move::fpromotion){
		//display();
		//sync_cout<<displayUci(m) <<sync_endl;
		captured=bitboardIndex(whiteQueens+m.promotion);
		swapList[0] +=seeScore[whiteQueens+m.promotion]-seeScore[whitePawns];
	}

	// Find all attackers to the destination square, with the moving piece
	// removed, but possibly an X-ray attacker added behind it.
	bitMap && attackers = getAttackersTo(to, occupied) & occupied;
//	displayBitMap(attackers);

	// If the opponent has no attackers we are finished
	color = (eNextMove)(blackTurn-color);
	assert(Pieces+color<lastBitboard);
	colorAttackers = attackers & bitBoard[Pieces+color];

//	displayBitMap(colorAttackers);

	if (!colorAttackers){
#ifdef DEBUG_SEE
	sync_cout<<"SEE:"<<swapList[0]/10000.0<<sync_endl;
#endif
		return swapList[0];
	}


	// The destination square is defended, which makes things rather more
	// difficult to compute. We proceed by building up a "swap list" containing
	// the material gain or loss at each stop in a sequence of captures to the
	// destination square, where the sides alternately capture, and always
	// capture with the least valuable piece. After each capture, we look for
	// new X-ray attacks from behind the capturing piece.

	assert(captured<lastBitboard);
	assert(squares[from]!=empty);
	do {
		assert(slIndex < 64);

		// Add the new entry to the swap list
		swapList[slIndex] = -swapList[slIndex - 1] + seeScore[captured];
		slIndex++;

		// Locate and remove the next least valuable attacker
		bitboardIndex nextAttacker= (bitboardIndex)(Pawns);
		while(nextAttacker>=King){
			bitMap att=bitBoard[nextAttacker+color] &colorAttackers;
//			displayBitMap(att);
			if(att){
				att= att & ~(att - 1); // find only one attacker
//				displayBitMap(att);
				occupied ^= att;
//				displayBitMap(occupied);
				attackers ^= att;
//				displayBitMap(attackers);

				if (nextAttacker == Pawns || nextAttacker == Bishops || nextAttacker == Queens){
					attackers |= Movegen::attackFromBishop(to,occupied)& (bitBoard[whiteBishops] |bitBoard[blackBishops] |bitBoard[whiteQueens] |bitBoard[blackQueens]);
				}

				if (nextAttacker == Rooks || nextAttacker == Queens){
					assert(to<squareNumber);
					attackers |= Movegen::attackFromRook(to,occupied)& (bitBoard[whiteRooks] |bitBoard[blackRooks] |bitBoard[whiteQueens] |bitBoard[blackQueens]);
				}
				attackers &= occupied;
//				displayBitMap(attackers);
				captured=nextAttacker;
				if(relativeRank==7 && captured==whitePawns){
					captured=whiteQueens;
				}
				break;
			}
			nextAttacker=bitboardIndex(nextAttacker-1);
		}
		//captured = min_attacker<PAWN>(byTypeBB, to, stmAttackers, occupied, attackers);


		color = (eNextMove)(blackTurn-color);
		colorAttackers = attackers & bitBoard[Pieces+color];
//		displayBitMap(colorAttackers);

		// Stop before processing a king capture
		if (captured == King && colorAttackers)
		{
			swapList[slIndex++] = seeScore[whiteKing];
			break;
		}

	} while (colorAttackers);

	// Having built the swap list, we negamax through it to find the best
	// achievable score from the point of view of the side to move.
	while (--slIndex){
		swapList[slIndex-1] = std::min(-swapList[slIndex], swapList[slIndex-1]);
	}

#ifdef DEBUG_SEE
	sync_cout<<"SEE:"<<swapList[0]/10000.0<<sync_endl;
#endif



	return swapList[0];











}
