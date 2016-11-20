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

#include <random>
#include <ctime>

#include <vector>
#include <list>
#include <algorithm>    // std::copy
#include <iterator>     // std::back_inserter
#include <atomic>
#include "search.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "history.h"
#include "book.h"
#include "thread.h"
#include "command.h"
#include "syzygy/tbprobe.h"
#include "bitops.h"


#ifdef DEBUG_EVAL_SIMMETRY
	Position ppp;
#endif

Search defaultSearch;
std::vector<rootMove> Search::rootMoves;
std::atomic<unsigned long long> Search::visitedNodes;
std::atomic<unsigned long long> Search::tbHits;
int Search::globalReduction =0;


Score Search::futility[8] = {0,6000,12000,18000,24000,30000,36000,42000};
Score Search::futilityMargin[7] = {0,10000,20000,30000,40000,50000,60000};
unsigned int Search::FutilityMoveCounts[11] = {5,10,17,26,37,50,66,85,105,130,151};
Score Search::PVreduction[32*ONE_PLY][64];
Score Search::nonPVreduction[32*ONE_PLY][64];
unsigned int Search::threads = 1;
unsigned int Search::multiPVLines = 1;
bool Search::useOwnBook = true;
bool Search::bestMoveBook = false;
bool Search::showCurrentLine = false;
std::string Search::SyzygyPath ="<empty>";
unsigned int Search::SyzygyProbeDepth = 1;
bool Search::Syzygy50MoveRule= true;



void Search::reloadPv( unsigned int i )
{
	if( rootMoves[i].PV.size() > 0)
	{
		auto PV = rootMoves[i].PV;
		unsigned int n = 0;

		for(const Move& m : PV)
		{
			if (!pos.isMoveLegal(m))
			{
				break;
			}

			const ttEntry* const tte = TT.probe(pos.getKey());

			if (!tte || tte->getPackedMove() != m.packed)
			{
				// Don't overwrite correct entries
				TT.store(pos.getKey(), SCORE_NONE, typeExact, -1000, m.packed, pos.eval<false>());
			}

			pos.doMove(m);
			n++;
		}

		for(unsigned int i = 0; i< n; i++)
		{
			pos.undoMove();
		}
	}
}

void Search::verifyPv(std::list<Move> &newPV, Score res)
{

	unsigned int n = 0;

	for(const Move& m : newPV)
	{
		if (!pos.isMoveLegal(m))
		{
			break;
		}
		pos.doMove(m);
		n++;
	}
	pos.display();
	Score temp = pos.eval<true>();
	sync_cout<<"EVAL:" <<temp/10000.0<<sync_endl;
	sync_cout<<"gamePhase:" <<pos.getGamePhase()/65536.0*100<<sync_endl;
	if( n %2 )
	{
		temp = -temp;
	}
	sync_cout<< temp << " " <<res<<sync_endl;

	for(unsigned int i = 0; i< n; i++)
	{
		pos.undoMove();
	}

}

startThinkResult Search::startThinking(int depth, Score alpha, Score beta)
{
	useTBresult = false;
	//------------------------------------
	//init the new search
	//------------------------------------
	Score res = 0;
	Score TBres = 0;


	TT.newSearch();
	history.clear();
	counterMoves.clear();
	cleanData();
	visitedNodes = 0;
	tbHits = 0;

	std::vector<Search> helperSearch(threads-1);

	rootMoves.clear();


	//--------------------------------
	//	generate the list of root moves to be searched
	//--------------------------------
	if(limits.searchMoves.size() == 0)	// all the legal moves
	{
		Move m(Movegen::NOMOVE);
		Movegen mg(pos);
		while ((m = mg.getNextMove())!= Movegen::NOMOVE)
		{
			rootMove rm;
			rm.init(m);
			rootMoves.push_back(rm);
		}

	}
	else
	{	//only selected moves
		for_each(limits.searchMoves.begin(), limits.searchMoves.end(),
			[&](Move &m)
			{
			rootMove rm;
			rm.init(m);
			rootMoves.push_back(rm);
			}
		);
	}





	//-----------------------------
	// manage multi PV moves
	//-----------------------------
	unsigned int linesToBeSearched = std::min(Search::multiPVLines, (unsigned int)rootMoves.size());

	//--------------------------------
	//	tablebase probing
	//--------------------------------
	if(limits.searchMoves.size() == 0 && Search::multiPVLines==1)
	{
		//sync_cout<<"ROOT PROBE"<<sync_endl;

		unsigned results[TB_MAX_MOVES];

		unsigned int piecesCnt = bitCnt (pos.getBitmap(Position::whitePieces) | pos.getBitmap(Position::blackPieces));

		if (    piecesCnt <= TB_LARGEST)
		{
			unsigned result = tb_probe_root(pos.getBitmap(Position::whitePieces),
				pos.getBitmap(Position::blackPieces),
				pos.getBitmap(Position::blackKing) | pos.getBitmap(Position::whiteKing),
				pos.getBitmap(Position::blackQueens) | pos.getBitmap(Position::whiteQueens),
				pos.getBitmap(Position::blackRooks) | pos.getBitmap(Position::whiteRooks),
				pos.getBitmap(Position::blackBishops) | pos.getBitmap(Position::whiteBishops),
				pos.getBitmap(Position::blackKnights) | pos.getBitmap(Position::whiteKnights),
				pos.getBitmap(Position::blackPawns) | pos.getBitmap(Position::whitePawns),
				pos.getActualState().fiftyMoveCnt,
				pos.getActualState().castleRights,
				pos.getActualState().epSquare == squareNone? 0 : pos.getActualState().epSquare ,
				pos.getActualState().nextMove== Position::whiteTurn,
				results);

			if (result != TB_RESULT_FAILED)
			{
				useTBresult= true;

				//sync_cout<<"endgame found"<<sync_endl;
				const unsigned wdl = TB_GET_WDL(result);
				assert(wdl<5);
				switch(wdl)
				{
				case 0:
					TBres = SCORE_MATED +100;
					//sync_cout<<"lost"<<sync_endl;
					break;
				case 1:
					TBres = -100;
					//sync_cout<<"blessed lost"<<sync_endl;
					break;
				case 2:
					TBres = 0;
					//sync_cout<<"draw"<<sync_endl;
					break;
				case 3:
					TBres = 100;
					sync_cout<<"cursed won"<<sync_endl;
					break;
				case 4:
					TBres = SCORE_MATE -100;
					//sync_cout<<"won"<<sync_endl;
					break;
				default:
					TBres = 0;
				}
				unsigned r;
				for (int i = 0; (r = results[i]) != TB_RESULT_FAILED; i++)
				{
					//sync_cout<<"MOSSA "<< i<<sync_endl;
					const unsigned moveWdl = TB_GET_WDL(r);

					unsigned ep = TB_GET_EP(r);
					Move m(0);
					m.bit.from = TB_GET_FROM(r);
					m.bit.to = TB_GET_TO(r);
					if (ep)
					{
						m.bit.flags = Move::fenpassant;
					}
					switch (TB_GET_PROMOTES(r))
					{
					case TB_PROMOTES_QUEEN:
						m.bit.flags = Move::fpromotion;
						m.bit.promotion = Move::promQueen;
						break;
					case TB_PROMOTES_ROOK:
						m.bit.flags = Move::fpromotion;
						m.bit.promotion = Move::promRook;
						break;
					case TB_PROMOTES_BISHOP:
						m.bit.flags = Move::fpromotion;
						m.bit.promotion = Move::promBishop;
						break;
					case TB_PROMOTES_KNIGHT:
						m.bit.flags = Move::fpromotion;
						m.bit.promotion = Move::promKnight;
						break;
					default:
						break;
					}



					auto position = std::find(rootMoves.begin(), rootMoves.end(), m);
					if (position != rootMoves.end()) // == myVector.end() means the element was not found
					{
						if (moveWdl >= wdl)
						{
							// preserve move
							//sync_cout<<displayUci(m)<<sync_endl;
						}
						else
						{
							//sync_cout<<"erase "<<displayUci(m)<<sync_endl;
							rootMoves.erase(position);
						}

					}
					else
					{
						sync_cout<<"ERRRRORE "<<displayUci(m)<<sync_endl;
						sync_cout<<m.packed<<sync_endl;
						sync_cout<<rootMoves.size()<<sync_endl;

					}

				}
			}
		}

	}
	//----------------------------------
	// we can start the real search
	//----------------------------------

	if(limits.depth == 0)
	{

		std::list<Move> newPV;
		Score res =qsearch<Search::nodeType::PV_NODE>(0, 0, -SCORE_INFINITE,SCORE_INFINITE, newPV);
		sync_cout<<"info score cp "<<int(res/100)<<sync_endl;

		startThinkResult ret;
		ret.PV = newPV;
		ret.depth = 0;
		ret.alpha = -SCORE_INFINITE;
		ret.beta = SCORE_INFINITE;


		return ret;

	}


	std::list<Move> newPV;
	//unsigned int depth = 1;

	//Score alpha = -SCORE_INFINITE, beta = SCORE_INFINITE;
	Score delta = 800;
	Move oldBestMove(Movegen::NOMOVE);

	do
	{
		sync_cout<<"info depth "<<depth<<sync_endl;
		//----------------------------
		// iterative loop
		//----------------------------
		for (rootMove& rm : rootMoves)
		{
			rm.previousScore = rm.score;
		}

		for (indexPV = 0; indexPV < linesToBeSearched; indexPV++)
		{

			//----------------------------------
			// prepare alpha & beta
			//----------------------------------

			for (unsigned int x = indexPV; x < rootMoves.size() ; x++)
			{
				rootMoves[x].score = -SCORE_INFINITE;
			}

			//----------------------------------
			// reload the last PV in the transposition table
			//----------------------------------
			for(unsigned int i = 0; i<=indexPV; i++)
			{
				reloadPv(i);
			}


			globalReduction = 0;

			do
			{

				//----------------------------
				// search at depth d with aspiration window
				//----------------------------

				maxPlyReached = 0;
				validIteration = false;

				//----------------------------
				// multithread : lazy smp threads
				//----------------------------

				std::vector<std::list<Move>> pvl2(threads-1);
				std::vector<std::thread> helperThread;

				// launch helper threads
				for(unsigned int i = 0; i < (threads - 1); i++)
				{
					helperSearch[i].stop = false;
					helperSearch[i].pos = pos;
					helperThread.push_back( std::thread(&Search::alphaBeta<Search::nodeType::HELPER_ROOT_NODE>, &helperSearch[i], 0, (depth-globalReduction+((i+1)%2))*ONE_PLY, alpha, beta, std::ref(pvl2[i])));
				}

				newPV.clear();
				// main thread
				res = alphaBeta<Search::nodeType::ROOT_NODE>(0, (depth-globalReduction) * ONE_PLY, alpha, beta, newPV);

				res = useTBresult ? TBres : res;
				// stop helper threads
				for(unsigned int i = 0; i< (threads - 1); i++)
				{
					helperSearch[i].stop = true;
				}
				for(auto &t : helperThread)
				{
					t.join();
				}

				// don't stop befor having finished at least one iteration
				/*if(depth != 1 && stop)
				{
					break;
				}*/

				if(validIteration ||!stop)
				{


					long long int elapsedTime  = getElapsedTime();

					assert(newPV.size()==0 || res >alpha);
					if( newPV.size() !=0 && res > alpha)
					{
						auto it = std::find(rootMoves.begin()+indexPV, rootMoves.end(), newPV.front() );

						if (it != rootMoves.end())
						{
							assert( it->firstMove == newPV.front());

							//if(it->firstMove == newPV.front())
							//{
							it->PV = newPV;
							it->score = res;
							it->maxPlyReached = maxPlyReached;
							it->depth = depth;
							it->nodes = visitedNodes;
							it->time = elapsedTime;

							std::iter_swap( it, rootMoves.begin()+indexPV);
						}
						else
						{
							//std::cout<<"info ERROR"<<sync_endl;
						}

						//}

					}






					if (res <= alpha)
					{
						newPV.clear();
						newPV.push_back( rootMoves[indexPV].PV.front() );

						printPV(res, depth, maxPlyReached, alpha, beta, elapsedTime, indexPV, newPV, visitedNodes,tbHits);

						alpha = (Score) std::max((signed long long int)(res) - delta, (signed long long int)-SCORE_INFINITE);

						globalReduction = 0;
						my_thread::timeMan.idLoopAlpha = true;
						my_thread::	timeMan.idLoopBeta = false;

					}
					else if (res >= beta)
					{

						printPV(res, depth, maxPlyReached, alpha, beta, elapsedTime, indexPV, newPV, visitedNodes,tbHits);

						beta = (Score) std::min((signed long long int)(res) + delta, (signed long long int)SCORE_INFINITE);
						if(depth > 1)
						{
							globalReduction = 1;
						}
						my_thread::timeMan.idLoopAlpha = false;
						my_thread::	timeMan.idLoopBeta = true;
					}
					else
					{
						//verify search result
						// print PV
						/*sync_cout<<"PV ";
						for_each( newPV.begin(), newPV.end(), [&](Move &m){std::cout<<displayUci(m)<<" ";});
						std::cout<<sync_endl;
						verifyPv(newPV,res);*/
						break;
					}


					delta += delta / 2;
				}



			}while(!stop);

			if((!stop || validIteration) && linesToBeSearched>1)
			{

				// Sort the PV lines searched so far and update the GUI
				std::stable_sort(rootMoves.begin(), rootMoves.begin() + indexPV + 1);
				printPVs( indexPV + 1 );
			}
		}



		//-----------------------
		//	single good move at root
		//-----------------------
		/*if (alpha > -11000 && beta <11000 && depth >= 12
			&& !stop
			&&  linesToBeSearched == 1
			&&  res > - SCORE_KNOWN_WIN)
		{
			for(int i = 9; i>=0;i--)
			{

				//unsigned long long v = visitedNodes;
				//sync_cout<<"SINGULAR MOVE SEARCH"<<sync_endl;
				Score rBeta = res - 20000+2000*i;
				sd[0].excludeMove = newPV.front();
				sd[0].skipNullMove = true;
				std::list<Move> locChildPV;
				Score temp = alphaBeta<Search::nodeType::ALL_NODE>(0, (depth-3) * ONE_PLY, rBeta - 1, rBeta, locChildPV);
				sd[0].skipNullMove = false;
				sd[0].excludeMove = Movegen::NOMOVE;

				if(temp < rBeta)
				{
					my_thread::timeMan.singularRootMoveCount++;
					//sync_cout<<"info debug SINGULAR MOVE "<<rBeta/100<<" "<<100.0*(visitedNodes-v)/float(visitedNodes)<<"% "<<my_thread::timeMan.singularRootMoveCount<<sync_endl;
				}
				else
				{
					if(i==9)
					{
						my_thread::timeMan.singularRootMoveCount = 0;
					}

					//sync_cout<<"info debug NO SINGULAR MOVE "<<rBeta/100<<" "<<100.0*(visitedNodes-v)/float(visitedNodes)<<"%"<<sync_endl;
					break;
				}
			}

		}*/

		//------------------------------------------------
		// check wheter or not the new best move has changed
		//------------------------------------------------
		oldBestMove = newPV.front();

		printPV(res, depth, maxPlyReached, alpha, beta, getElapsedTime(), 0, newPV, visitedNodes,tbHits);

		my_thread::timeMan.idLoopIterationFinished = true;
		my_thread::timeMan.idLoopAlpha = false;
		my_thread::	timeMan.idLoopBeta = false;
		depth += 1;

	}
	while( depth <= (limits.depth != -1 ? limits.depth : 100) && !stop);


	startThinkResult ret;
	ret.PV = rootMoves[0].PV;
	ret.depth = depth-1;
	ret.alpha = alpha;
	ret.beta = beta;


	return ret;

}

template<Search::nodeType type> Score Search::alphaBeta(unsigned int ply, int depth, Score alpha, Score beta, std::list<Move>& pvLine)
{
	//sync_cout<<"alpha beta"<<sync_endl;

	assert(alpha <beta);
	assert(alpha>=-SCORE_INFINITE);
	assert(beta<=SCORE_INFINITE);
	assert(depth>=ONE_PLY);

	//visitedNodes++;

	const bool PVnode = ( type == Search::nodeType::PV_NODE || type == Search::nodeType::ROOT_NODE  || type == Search::nodeType::HELPER_ROOT_NODE);
	const bool inCheck = pos.isInCheck();



	if(type != Search::nodeType::ROOT_NODE  && type !=Search::nodeType::HELPER_ROOT_NODE)
	{
		if(pos.isDraw(PVnode) || stop)
		{
			if(PVnode)
			{
				pvLine.clear();
			}
			return 0;
		}

	}
	U64 posKey =  pos.getKey();

	//--------------------------------------
	// test the transposition table
	//--------------------------------------
	ttEntry* tte = TT.probe(posKey);
	Move ttMove;

	ttMove = (tte != nullptr) ? tte->getPackedMove() : 0;






	Score bestScore = -SCORE_INFINITE;

	Move bestMove(Movegen::NOMOVE);

	Move m;
	Movegen mg(pos, *this, ply, ttMove);
	unsigned int moveNumber = 0;



	while (bestScore <beta  && ( m = mg.getNextMove() ) != Movegen::NOMOVE)
	{

		assert(m.packed);

		moveNumber++;

		int newDepth = depth-ONE_PLY;




		if(type == ROOT_NODE)
		{
			long long int elapsed = getElapsedTime();
			if(
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
				elapsed>3000 &&
#endif
				!stop
				)
			{
				printCurrMoveNumber(moveNumber, m, visitedNodes, elapsed);
			}
		}


		pos.doMove(m);
		Score val;
		std::list<Move> childPV;

		if(newDepth < ONE_PLY)
		{
			val = -qsearch<Search::nodeType::PV_NODE>(ply+1, newDepth, -beta, -alpha, childPV);
		}
		else
		{
			val = -alphaBeta<Search::nodeType::PV_NODE>(ply+1, newDepth, -beta, -alpha, childPV);
		}
		pos.undoMove();

		if(!stop && val > bestScore)
		{
			bestScore = val;
			if (bestScore >alpha)
			{
				alpha = bestScore;
				bestMove = m;
				pvLine.clear();
				pvLine.push_back(bestMove);
				pvLine.splice(pvLine.end(),childPV);
				if(type == Search::nodeType::ROOT_NODE && Search::multiPVLines==1)
				{
					printPV(val, depth/ONE_PLY+globalReduction, maxPlyReached, -SCORE_INFINITE, SCORE_INFINITE, getElapsedTime(), indexPV, pvLine, visitedNodes,tbHits);
					validIteration = true;
				}
			}


		}
	}


	// draw or mated

	if(!moveNumber)
	{
		if(!inCheck)
		{
			bestScore = std::min( (int)0, (int)(-5000 + pos.getPly()*250) );
		}
		else
		{
			bestScore = matedIn(ply);
		}
	}

	if (bestScore == -SCORE_INFINITE)
		bestScore = alpha;

	if(!stop)
	{
		TT.store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta  ? typeScoreHigherThanBeta :
					(PVnode && bestMove.packed) ? typeExact : typeScoreLowerThanAlpha,
							(short int)depth, bestMove.packed, 0);
	}
	return bestScore;

}


template<Search::nodeType type> Score Search::qsearch(unsigned int ply, int depth, Score alpha, Score beta, std::list<Move>& pvLine)
{
	//sync_cout<<alpha<<" "<<beta<<" "<<depth<<sync_endl;

	/*if( ply >=50)
	{
		sync_cout<< alpha<<" "<<beta<<" ";
		for(unsigned int i = 1;i<ply;i++)
		{
			std::cout<<displayUci(pos.getState(i).currentMove)<<" ";
		}

		std::cout<<sync_endl;
	}*/

	assert(ply>0);
	assert(depth<ONE_PLY);
	assert(alpha<beta);
	assert(beta<=SCORE_INFINITE);
	assert(alpha>=-SCORE_INFINITE);

	const bool PVnode = (type == Search::nodeType::PV_NODE);
	assert(PVnode || alpha+1==beta);

	bool inCheck = pos.isInCheck();
	unsigned int moveCount = 0;

	maxPlyReached = std::max(ply, maxPlyReached);
	visitedNodes++;


	if(pos.isDraw(PVnode) || stop)
	{

		if(PVnode)
		{
			pvLine.clear();
		}
		return 0;
	}


	//----------------------------
	//	next node type
	//----------------------------
	const Search::nodeType childNodesType = Search::nodeType::PV_NODE;


	ttEntry* const tte = TT.probe(pos.getKey());
	Move ttMove;
	ttMove = tte ? tte->getPackedMove() : Movegen::NOMOVE;

	Movegen mg(pos, *this, ply, ttMove);
	mg.setupQuiescentSearch(inCheck);



	Score staticEval = pos.eval<false>();
	//----------------------------
	//	stand pat score
	//----------------------------
	Score bestScore = staticEval;
	if( inCheck )
	{
		bestScore = -SCORE_INFINITE;
	}

	if(!inCheck && bestScore>alpha)
	{
		alpha = bestScore;
	}

//	return bestScore;
	//----------------------------
	//	try the captures
	//----------------------------
	Move m;
	Move bestMove = ttMove;

	std::list<Move> childPV;

	while (/*bestScore < beta  &&  */(m = mg.getNextMove()) != Movegen::NOMOVE)
	{
		assert(alpha < beta);
		assert(beta <= SCORE_INFINITE);
		assert(alpha >= -SCORE_INFINITE);
		assert(m.packed);

		moveCount++;

		pos.doMove(m);
		Score val = -qsearch<childNodesType>(ply+1, depth-ONE_PLY, -beta, -alpha, childPV);


		pos.undoMove();


		if( val > bestScore)
		{
			bestScore = val;

			if( bestScore> alpha )
			{
				bestMove = m;
				alpha = bestScore;


				if(PVnode && !stop)
				{
					pvLine.clear();
					pvLine.push_back(bestMove);
					pvLine.splice( pvLine.end(), childPV );

				}
				if( bestScore >= beta)
				{
					if(!stop)
					{
						TT.store(pos.getKey(), transpositionTable::scoreToTT(bestScore, ply), typeScoreHigherThanBeta,(short int)0, bestMove.packed, 0);
					}
					return bestScore;
				}
			}

		}
	}

	if(!moveCount)
	{
		pvLine.clear();
		if(inCheck)
		{
			bestScore = matedIn(ply);
		}
	}


	return bestScore;

}


