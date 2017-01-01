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

//---------------------------------------------
//	include
//---------------------------------------------
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <iterator>
#include "vajolet.h"
#include "command.h"
#include "io.h"
#include "position.h"
#include "movegen.h"
#include "move.h"
#include "search.h"
#include "thread.h"
#include "transposition.h"
#include "benchmark.h"
#include "syzygy/tbprobe.h"




//---------------------------------------------
//	local global constant
//---------------------------------------------

const static std::string StartFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
/*! \brief array of char to create the fen string
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/



//---------------------------------------------
//	function implementation
//---------------------------------------------

/*	\brief print the uci reply
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void static printUciInfo(void)
{
	sync_cout << "id name " << PROGRAM_NAME << " " << VERSION << sync_endl;
	sync_cout << "id author Belli Marco" << sync_endl;
	sync_cout << "option name Hash type spin default 1 min 1 max 65535" << sync_endl;
	sync_cout << "option name Threads type spin default 1 min 1 max 128" << sync_endl;
	sync_cout << "option name MultiPV type spin default 1 min 1 max 500" << sync_endl;
	sync_cout << "option name Ponder type check default true" << sync_endl;
	sync_cout << "option name OwnBook type check default true" <<sync_endl;
	sync_cout << "option name BestMoveBook type check default false"<<sync_endl;
	sync_cout << "option name UCI_EngineAbout type string default VajoletII by Marco Belli" << sync_endl;
	sync_cout << "option name UCI_ShowCurrLine type check default false" << sync_endl;
	sync_cout << "option name SyzygyPath type string default <empty>" << sync_endl;
	sync_cout << "option name SyzygyProbeDepth type spin default 1 min 1 max 100" << sync_endl;
	sync_cout << "option name isolatedPawnPenaltyOp type spin default 1 min 0 max 10000" << sync_endl;
	sync_cout << "option name isolatedPawnPenaltyEn type spin default 1 min 0 max 10000" << sync_endl;

	sync_cout << "uciok" << sync_endl;
}


/*	\brief get an input string and convert it to a valid move;
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
Move moveFromUci(const Position& pos,const  std::string& str)
{

	// idea from stockfish, we generate all the legal moves and return the legal moves with the same UCI string
	Move m;
	Movegen mg(pos);
	while( (m = mg.getNextMove()).packed)
	{
		if(str == displayUci(m))
		{
			return m;
		}
	}
	// move not found
	return Movegen::NOMOVE;
}


/*	\brief handle position command
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void static position(std::istringstream& is, Position & pos)
{
	std::string token, fen;
	is >> token;
	if (token == "startpos")
	{
		fen = StartFEN;
		is >> token; // Consume "moves" token if any
	}
	else if (token == "fen")
	{
		while (is >> token && token != "moves")
		{
			fen += token + " ";
		}
	}
	else
	{
		return;
	}

	pos.setupFromFen(fen);

	Move m;
	// Parse move list (if any)
	while (is >> token && ((m = moveFromUci(pos, token)) != Movegen::NOMOVE))
	{
		pos.doMove(m);
	}
}

/*	\brief handle perft command
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
void static doPerft(const unsigned int n, Position & pos)
{

	unsigned long long elapsed = Search::getTime();
	unsigned long long res = pos.perft(n);
	elapsed = Search::getTime() - elapsed;

	sync_cout << "Perft " << n << " leaf nodes: " << res << sync_endl;
	sync_cout << elapsed << "ms " << ((double)res) / (double)elapsed << " kN/s" << sync_endl;
}


void static go(std::istringstream& is, Position & pos, my_thread * thr)
{
	searchLimits limits;
	std::string token;
	bool searchMovesCommand = false;


    while (is >> token)
    {
        if (token == "searchmoves")		{searchMovesCommand = true;}
        else if (token == "wtime")		{is >> limits.wtime;searchMovesCommand = false;}
        else if (token == "btime")		{is >> limits.btime;searchMovesCommand = false;}
        else if (token == "winc")		{is >> limits.winc;searchMovesCommand = false;}
        else if (token == "binc")		{is >> limits.binc;searchMovesCommand = false;}
        else if (token == "movestogo")	{is >> limits.movesToGo;searchMovesCommand = false;}
        else if (token == "depth")		{is >> limits.depth;searchMovesCommand = false;}
        else if (token == "nodes")		{is >> limits.nodes;searchMovesCommand = false;}
        else if (token == "movetime")	{is >> limits.moveTime;searchMovesCommand = false;}
        else if (token == "mate")		{is >> limits.mate;searchMovesCommand = false;}
        else if (token == "infinite")	{limits.infinite = true;searchMovesCommand = false;}
        else if (token == "ponder")		{limits.ponder = true;searchMovesCommand = false;}
        else if (searchMovesCommand == true)
        {
        	Move m = moveFromUci(pos, token);
			if(m.packed)
			{
				sync_cout << "mossa " << token << sync_endl;
				limits.searchMoves.push_back( moveFromUci(pos, token) );
			}
        }
    }
    thr->startThinking(&pos,limits);
}




void setoption(std::istringstream& is)
{
	std::string token, name, value;

	is >> token; // Consume "name" token

	// Read option name (can contain spaces)
	while (is >> token && token != "value")
	{
		name += std::string(" ", !name.empty()) + token;
	}
	// Read option value (can contain spaces)
	while (is >> token)
	{
		value += std::string(" ", !value.empty()) + token;
	}

	if(name == "Hash")
	{
		int hash = stoi(value);
		hash = std::min(hash,65536);
		TT.setSize(hash);
	}
	else if(name == "Threads")
	{
		int i = stoi(value);
		Search::threads = (i<=128)?(i>0?i:1):128;
	}
	else if(name == "MultiPV")
	{
		int i = stoi(value);
		Search::multiPVLines = i<500 ? (i>0 ? i : 1) : 500;
	}
	else if(name == "OwnBook")
	{
		if(value=="true")
		{
			Search::useOwnBook = true;
		}
		else{
			Search::useOwnBook = false;
		}
	}
	else if(name == "BestMoveBook")
	{
		if(value == "true")
		{
			Search::bestMoveBook = true;
		}
		else
		{
			Search::bestMoveBook = false;
		}
	}
	else if(name == "UCI_ShowCurrLine")
	{
		if(value == "true"){
			Search::showCurrentLine = true;
		}
		else
		{
			Search::showCurrentLine = false;
		}
	}
	else if(name == "Ponder")
	{
	}
	else if(name == "SyzygyPath")
	{
		Search::SyzygyPath = value;
		tb_init(Search::SyzygyPath.c_str());
		sync_cout<<"info string TB_LARGEST = "<<TB_LARGEST<<sync_endl;
	}
	else if(name == "SyzygyProbeDepth")
	{
		Search::SyzygyProbeDepth = stoi(value);
	}
	else if(name == "Syzygy50MoveRule")
	{
		if(value == "true")
		{
			Search::Syzygy50MoveRule = true;
		}
		else
		{
			Search::Syzygy50MoveRule = false;
		}
	}
	else if(name == "isolatedPawnPenaltyOp")
	{
		int i = stoi(value);
		isolatedPawnPenalty[0] = i;
		sync_cout<<"info string ReceivedPar isolatedPawnPenaltyOp = "<<i<<sync_endl;
	}
	else if(name == "isolatedPawnPenaltyEn")
	{
		int i = stoi(value);
		isolatedPawnPenalty[1] = i;
		sync_cout<<"info string ReceivedPar isolatedPawnPenaltyEn = "<<i<<sync_endl;
	}
	else if(name == "isolatedPawnPenaltyOppOp")
	{
		int i = stoi(value);
		isolatedPawnPenaltyOpp[0] = i;
		sync_cout<<"info string ReceivedPar isolatedPawnPenaltyOppOp = "<<i<<sync_endl;
	}
	else if(name == "isolatedPawnPenaltyOppEn")
	{
		int i = stoi(value);
		isolatedPawnPenaltyOpp[1] = i;
		sync_cout<<"info string ReceivedPar isolatedPawnPenaltyOppEn = "<<i<<sync_endl;
	}
	else if(name == "doubledPawnPenaltyOp")
	{
		int i = stoi(value);
		doubledPawnPenalty[0] = i;
		sync_cout<<"info string ReceivedPar doubledPawnPenaltyOp = "<<i<<sync_endl;
	}
	else if(name == "doubledPawnPenaltyEn")
	{
		int i = stoi(value);
		doubledPawnPenalty[1] = i;
		sync_cout<<"info string ReceivedPar doubledPawnPenaltyEn = "<<i<<sync_endl;
	}
	else if(name == "backwardPawnPenaltyOp")
	{
		int i = stoi(value);
		backwardPawnPenalty[0] = i;
		sync_cout<<"info string ReceivedPar backwardPawnPenaltyOp = "<<i<<sync_endl;
	}
	else if(name == "backwardPawnPenaltyEn")
	{
		int i = stoi(value);
		backwardPawnPenalty[1] = i;
		sync_cout<<"info string ReceivedPar backwardPawnPenaltyEn = "<<i<<sync_endl;
	}
	else if(name == "chainedPawnBonusOp")
	{
		int i = stoi(value);
		chainedPawnBonus[0] = i;
		sync_cout<<"info string ReceivedPar chainedPawnBonusOp = "<<i<<sync_endl;
	}
	else if(name == "chainedPawnBonusEn")
	{
		int i = stoi(value);
		chainedPawnBonus[1] = i;
		sync_cout<<"info string ReceivedPar chainedPawnBonusEn = "<<i<<sync_endl;
	}
	else if(name == "passedPawnFileAHPenaltyOp")
	{
		int i = stoi(value);
		passedPawnFileAHPenalty[0] = i;
		sync_cout<<"info string ReceivedPar passedPawnFileAHPenaltyOp = "<<i<<sync_endl;
	}
	else if(name == "passedPawnFileAHPenaltyEn")
	{
		int i = stoi(value);
		passedPawnFileAHPenalty[1] = i;
		sync_cout<<"info string ReceivedPar passedPawnFileAHPenaltyEn = "<<i<<sync_endl;
	}
	else if(name == "passedPawnSupportedBonusOp")
	{
		int i = stoi(value);
		passedPawnSupportedBonus[0] = i;
		sync_cout<<"info string ReceivedPar passedPawnSupportedBonusOp = "<<i<<sync_endl;
	}
	else if(name == "passedPawnSupportedBonusEn")
	{
		int i = stoi(value);
		passedPawnSupportedBonus[1] = i;
		sync_cout<<"info string ReceivedPar passedPawnSupportedBonusEn = "<<i<<sync_endl;
	}
	else if(name == "candidateBonusOp")
	{
		int i = stoi(value);
		candidateBonus[0] = i;
		sync_cout<<"info string ReceivedPar candidateBonusOp = "<<i<<sync_endl;
	}
	else if(name == "candidateBonusEn")
	{
		int i = stoi(value);
		candidateBonus[1] = i;
		sync_cout<<"info string ReceivedPar candidateBonusEn = "<<i<<sync_endl;
	}
	else if(name == "passedPawnBonusOp")
	{
		int i = stoi(value);
		passedPawnBonus[0] = i;
		sync_cout<<"info string ReceivedPar passedPawnBonusOp = "<<i<<sync_endl;
	}
	else if(name == "passedPawnBonusEn")
	{
		int i = stoi(value);
		passedPawnBonus[1] = i;
		sync_cout<<"info string ReceivedPar passedPawnBonusEn = "<<i<<sync_endl;
	}
	else if(name == "passedPawnUnsafeSquaresOp")
	{
		int i = stoi(value);
		passedPawnUnsafeSquares[0] = i;
		sync_cout<<"info string ReceivedPar passedPawnUnsafeSquaresOp = "<<i<<sync_endl;
	}
	else if(name == "passedPawnUnsafeSquaresEn")
	{
		int i = stoi(value);
		passedPawnUnsafeSquares[1] = i;
		sync_cout<<"info string ReceivedPar passedPawnUnsafeSquaresEn = "<<i<<sync_endl;
	}
	else if(name == "passedPawnBlockedSquaresOp")
	{
		int i = stoi(value);
		passedPawnBlockedSquares[0] = i;
		sync_cout<<"info string ReceivedPar passedPawnBlockedSquaresOp = "<<i<<sync_endl;
	}
	else if(name == "passedPawnBlockedSquaresEn")
	{
		int i = stoi(value);
		passedPawnBlockedSquares[1] = i;
		sync_cout<<"info string ReceivedPar passedPawnBlockedSquaresEn = "<<i<<sync_endl;
	}
	else if(name == "passedPawnDefendedSquaresOp")
	{
		int i = stoi(value);
		passedPawnDefendedSquares[0] = i;
		sync_cout<<"info string ReceivedPar passedPawnDefendedSquaresOp = "<<i<<sync_endl;
	}
	else if(name == "passedPawnDefendedSquaresEn")
	{
		int i = stoi(value);
		passedPawnDefendedSquares[1] = i;
		sync_cout<<"info string ReceivedPar passedPawnDefendedSquaresEn = "<<i<<sync_endl;
	}
	else if(name == "passedPawnDefendedBlockingSquareOp")
	{
		int i = stoi(value);
		passedPawnDefendedBlockingSquare[0] = i;
		sync_cout<<"info string ReceivedPar passedPawnDefendedBlockingSquareOp = "<<i<<sync_endl;
	}
	else if(name == "passedPawnDefendedBlockingSquareEn")
	{
		int i = stoi(value);
		passedPawnDefendedBlockingSquare[1] = i;
		sync_cout<<"info string ReceivedPar passedPawnDefendedBlockingSquareEn = "<<i<<sync_endl;
	}
	else if(name == "unstoppablePassedOp")
	{
		int i = stoi(value);
		unstoppablePassed[0] = i;
		sync_cout<<"info string ReceivedPar unstoppablePassedOp = "<<i<<sync_endl;
	}
	else if(name == "unstoppablePassedEn")
	{
		int i = stoi(value);
		unstoppablePassed[1] = i;
		sync_cout<<"info string ReceivedPar unstoppablePassedEn = "<<i<<sync_endl;
	}
	else if(name == "rookBehindPassedPawnOp")
	{
		int i = stoi(value);
		rookBehindPassedPawn[0] = i;
		sync_cout<<"info string ReceivedPar rookBehindPassedPawnOp = "<<i<<sync_endl;
	}
	else if(name == "rookBehindPassedPawnEn")
	{
		int i = stoi(value);
		rookBehindPassedPawn[1] = i;
		sync_cout<<"info string ReceivedPar rookBehindPassedPawnEn = "<<i<<sync_endl;
	}
	else if(name == "EnemyRookBehindPassedPawnOp")
	{
		int i = stoi(value);
		EnemyRookBehindPassedPawn[0] = i;
		sync_cout<<"info string ReceivedPar EnemyRookBehindPassedPawnOp = "<<i<<sync_endl;
	}
	else if(name == "EnemyRookBehindPassedPawnEn")
	{
		int i = stoi(value);
		EnemyRookBehindPassedPawn[1] = i;
		sync_cout<<"info string ReceivedPar EnemyRookBehindPassedPawnEn = "<<i<<sync_endl;
	}
	else
	{
		sync_cout << "No such option: " << name << sync_endl;
	}

}


/*
void setvalue(std::istringstream& is)
{
	std::string token, name, value;

	is >> name;

	is >> value;

	if(name =="queenMG")
	{
		Position::pieceValue[Position::whiteQueens].insert(0,stoi(value));
		Position::pieceValue[Position::blackQueens].insert(0,stoi(value));
		Position::initPstValues();
	}else if(name =="queenEG")
	{
		Position::pieceValue[Position::whiteQueens].insert(1,stoi(value));
		Position::pieceValue[Position::blackQueens].insert(1,stoi(value));
		Position::initPstValues();
	}else if(name =="rookMG")
	{
		Position::pieceValue[Position::whiteRooks].insert(0,stoi(value));
		Position::pieceValue[Position::blackRooks].insert(0,stoi(value));
		Position::initPstValues();
	}else if(name =="rookEG")
	{
		Position::pieceValue[Position::whiteRooks].insert(1,stoi(value));
		Position::pieceValue[Position::blackRooks].insert(1,stoi(value));
		Position::initPstValues();
	}else if(name =="bishopMG")
	{
		Position::pieceValue[Position::whiteBishops].insert(0,stoi(value));
		Position::pieceValue[Position::blackBishops].insert(0,stoi(value));
		Position::initPstValues();
	}else if(name =="bishopEG")
	{
		Position::pieceValue[Position::whiteBishops].insert(1,stoi(value));
		Position::pieceValue[Position::blackBishops].insert(1,stoi(value));
		Position::initPstValues();
	}else if(name =="knightMG")
	{
		Position::pieceValue[Position::whiteKnights].insert(0,stoi(value));
		Position::pieceValue[Position::blackKnights].insert(0,stoi(value));
		Position::initPstValues();
	}else if(name =="knightEG")
	{
		Position::pieceValue[Position::whiteKnights].insert(1,stoi(value));
		Position::pieceValue[Position::blackKnights].insert(1,stoi(value));
		Position::initPstValues();
	}else if(name =="pawnsEG")
	{
		Position::pieceValue[Position::whitePawns].insert(1,stoi(value));
		Position::pieceValue[Position::blackPawns].insert(1,stoi(value));
		Position::initPstValues();
	}
}*/


/*	\brief manage the uci loop
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void uciLoop()
{
	my_thread *thr = my_thread::getInstance();
	Position pos;
	pos.setupFromFen(StartFEN);
	std::string token, cmd;

	do
	{
		assert(thr);
		if (!std::getline(std::cin, cmd)) // Block here waiting for input
			cmd = "quit";
		std::istringstream is(cmd);

		is >> std::skipws >> token;


		if(token== "uci")
		{
			printUciInfo();
		}
		else if (token == "quit" || token == "stop")
		{
			thr->stopThinking();
		}
		else if (token == "ucinewgame")
		{
		}
		else if (token == "d")
		{
			pos.display();
		}
		else if (token == "position")
		{
			position(is,pos);
		}
		else if(token == "setoption")
		{
			setoption(is);
		}
		/*else if(token == "setvalue")
		{
			setvalue(is);
		}*/
		else if (token == "eval")
		{
			sync_cout << "Eval:" << pos.eval<true>() / 10000.0 << sync_endl;
			sync_cout << "gamePhase:"  << pos.getGamePhase()/65536.0*100 << "%" << sync_endl;
#ifdef DEBUG_EVAL_SIMMETRY

			Position ppp;
			ppp.setupFromFen(pos.getSymmetricFen());
			ppp.display();
			sync_cout << "Eval:"  << ppp.eval<true>() / 10000.0 << sync_endl;
			sync_cout << "gamePhase:" << ppp.getGamePhase()/65536.0*100 << "%" << sync_endl;

#endif

		}
		else if (token == "isready")
		{
			sync_cout << "readyok" << sync_endl;
		}
		else if (token == "perft" && (is>>token))
		{
			doPerft(stoi(token), pos);
		}
		else if (token == "divide" && (is>>token))
		{
			unsigned long long res = pos.divide(stoi(token));
			sync_cout << "divide Res= " << res << sync_endl;
		}
		else if (token == "go")
		{
			go(is, pos, thr);
		}
		else if (token == "bench")
		{
			benchmark();
		}
		else if (token == "ponderhit")
		{
			thr->ponderHit();
		}
		else
		{
			sync_cout << "unknown command" << sync_endl;
		}
	}while(token!="quit");

	thr->quitThreads();
}

/*! \brief return the uci string for a given move
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
std::string displayUci(const Move & m){


	std::string s;

	if(m.packed==0)
	{
		s = "0000";
		return s;
	}

	//from
	s += char('a'+FILES[m.bit.from]);
	s += char('1'+RANKS[m.bit.from]);


	//to
	s += char('a'+FILES[m.bit.to]);
	s += char('1'+RANKS[m.bit.to]);
	//promotion
	if(m.bit.flags == Move::fpromotion)
	{
		s += PIECE_NAMES_FEN[m.bit.promotion+Position::blackQueens];
	}
	return s;

}

/*! \brief return the uci string for a given move
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
std::string displayMove(const Position& pos, const Move & m)
{

	std::string s;

	bool capture = (bitSet((tSquare)m.bit.to) & pos.getTheirBitmap(Position::Pieces));

	if( !pos.isPawn(pos.getPieceAt( (tSquare)m.bit.from)) )
	{
		s+=PIECE_NAMES_FEN[pos.getPieceAt((tSquare)m.bit.from) % Position::separationBitmap];
	}
	if(capture && pos.isPawn(pos.getPieceAt((tSquare)m.bit.from)))
	{
		s+=char('a'+FILES[m.bit.from]);
	}
	if(capture)
	{
		s+="x";
	}




	//to
	s += char('a'+FILES[ m.bit.to ]);
	s += char('1'+RANKS[ m.bit.to ]);
	if( pos.moveGivesCheck(m) )
	{
		s+="+";
	}
	//promotion
	if(m.bit.flags == Move::fpromotion)
	{
		s += "=";
		s += PIECE_NAMES_FEN[m.bit.promotion + Position::whiteQueens];
	}
	return s;

}

void printCurrMoveNumber(unsigned int moveNumber, const Move &m, unsigned long long visitedNodes, long long int time)
{
	sync_cout << "info currmovenumber " << moveNumber << " currmove " << displayUci(m) << " nodes " << visitedNodes <<
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			" time " << time <<
#endif
	sync_endl;
}

void showCurrLine(const Position & pos, unsigned int ply)
{
	sync_cout << "info currline";
	unsigned int start = pos.getStateIndex()-ply + 1;

	for (unsigned int i = start; i<= start+ply/2; i++) // show only half of the search line
	{
		std::cout << " " << displayUci(pos.getState(i).currentMove);
	}
	std::cout << sync_endl;

}


void printPVs(unsigned int count)
{

	int i= 0;
	std::for_each(Search::rootMoves.begin(),std::next(Search::rootMoves.begin(), count), [&](rootMove& rm)
	{
		if(rm.nodes)
		{
			printPV(rm.score, rm.depth, rm.maxPlyReached, -SCORE_INFINITE, SCORE_INFINITE, rm.time, i, rm.PV, rm.nodes,Search::tbHits);
		}
		i++;
	});
}

void printPV(Score res,unsigned int depth,unsigned int seldepth,Score alpha, Score beta, long long time,unsigned int count,std::list<Move>& PV,unsigned long long nodes, unsigned long long tbHits)
{

	sync_cout<<"info multipv "<< (count+1) << " depth "<< (depth) <<" seldepth "<< seldepth <<" score ";

	if(abs(res) >SCORE_MATE_IN_MAX_PLY)
	{
		std::cout << "mate " << (res > 0 ? SCORE_MATE - res + 1 : -SCORE_MATE - res) / 2;
	}
	else
	{
		int satRes = std::min(res,SCORE_MAX_OUTPUT_VALUE);
		satRes = std::max(satRes,SCORE_MIN_OUTPUT_VALUE);
		std::cout << "cp "<< (int)((float)satRes/100.0);
	}

	std::cout << (res >= beta ? " lowerbound" : res <= alpha ? " upperbound" : "");

	std::cout << " nodes " << nodes <<" tbhits "<<tbHits;
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
	std::cout << " nps " << (unsigned int)((double)nodes*1000/(double)time) << " time " << (long long int)(time);
#endif

	std::cout << " pv ";
	for_each( PV.begin(), PV.end(), [&](Move &m){std::cout<<displayUci(m)<<" ";});
	std::cout<<sync_endl;
}
