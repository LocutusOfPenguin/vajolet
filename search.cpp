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
#include <chrono>
#include <vector>
#include <algorithm>    // std::copy
#include <iterator>     // std::back_inserter
#include "search.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "statistics.h"
#include "history.h"
#include "book.h"
#include "thread.h"


inline signed int razorMargin(unsigned int depth){
	return 20000+depth*78;
}

void search::printAllPV(Position & p, unsigned int count){


	for (unsigned int i=0; i<count; i++){
		Score res=rootMoves[i].previousScore;
		if(rootMoves[i].nodes)
		{
			printPV(res,rootMoves[i].depth,rootMoves[i].selDepth,-SCORE_INFINITE,SCORE_INFINITE,p,rootMoves[i].time,i,rootMoves[i].PV,rootMoves[i].nodes);
		}
	}
}

void search::printPV(Score res,unsigned int depth,unsigned int seldepth,Score alpha, Score beta, Position & p, unsigned long time,unsigned int count,std::vector<Move>& PV,unsigned long long nodes){
	sync_cout<<"info multipv "<< count+1<< " depth "<<(depth)<<" seldepth "<<seldepth <<" score ";
	if(abs(res) >SCORE_MATE_IN_MAX_PLY){
		std::cout << "mate " << (res > 0 ? SCORE_MATE - res + 1 : -SCORE_MATE - res) / 2;
	}
	else{
		std::cout<< "cp "<<(int)((float)res/100.0);
	}
	std::cout<<(res >= beta ? " lowerbound" : res <= alpha ? " upperbound" : "");


	std::cout<<" nodes "<<nodes;
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
	std::cout<<" nps "<<(unsigned int)((double)nodes*1000/(time))<<" time "<<(time);
#endif
	std::cout<<" pv ";
	for (auto it= PV.begin(); it != PV.end(); ++it){
		std::cout<<p.displayUci(*it)<<" ";
	}
	std::cout<<sync_endl;
}


Score search::futility[5]={0,6000,20000,30000,40000};
Score search::FutilityMoveCounts[11]={5,10,17,26,37,50,66,85,105,130,151};
Score search::PVreduction[32*ONE_PLY][64];
Score search::nonPVreduction[32*ONE_PLY][64];
unsigned int search::multiPVLines=1;
unsigned int search::limitStrength=0;
unsigned int search::eloStrenght=3000;
bool search::useOwnBook=true;
bool search::bestMoveBook=false;
bool search::showCurrentLine=false;

Score search::startThinking(Position & p,searchLimits & l){
	visitedNodes=0;
	Score res=0;
	signals.stop=false;
	TT.newSearch();
	history.clear();

	limits=l;
	rootMoves.clear();
	//--------------------------------
	//	generate the list of root moves to be searched
	//--------------------------------
	if(limits.searchMoves.size()==0){
		Move m;
		m=0;
		Movegen mg(p,history,m);
		while ((m=mg.getNextMove()).packed){
			rootMove rm;
			rm.previousScore=-SCORE_INFINITE;
			rm.score=-SCORE_INFINITE;
			rm.firstMove=m;
			rm.selDepth=0;
			rm.depth=0;
			rm.nodes=0;
			rm.time=0;

			rootMoves.push_back(rm);
		}
	}
	else{
		for(std::list<Move>::iterator it = limits.searchMoves.begin(); it != limits.searchMoves.end(); ++it){
			rootMove rm;
			rm.previousScore=-SCORE_INFINITE;
			rm.score=-SCORE_INFINITE;
			rm.firstMove=*it;
			rm.selDepth=0;
			rm.depth=0;
			rm.nodes=0;
			rm.time=0;
			rootMoves.push_back(rm);
		}
	}

	//sync_cout<<"legal moves ="<<rootMoves.size()<<sync_endl;
	unsigned int PVSize = search::multiPVLines;

	if(limitStrength){
		int lines= (-8.0/2000.0)*(eloStrenght-1000)+10.0;
		unsigned int linesToBeSearched=std::max(lines,4);
		PVSize=	std::max(PVSize,linesToBeSearched);
	}
	PVSize = std::min(PVSize, (unsigned int)rootMoves.size());
	//sync_cout<<"PV_SIZE ="<<PVSize<<sync_endl;
	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/
	Move m,oldBestMove;
	m=0;
	oldBestMove=0;
	Movegen mg(p,history,m);

	Move lastLegalMove;
	unsigned int legalMoves=0;
	while((m=mg.getNextMove()).packed){
		legalMoves++;
		lastLegalMove=m;
	}
	if(legalMoves==0){
		while((limits.infinite && !signals.stop) || limits.ponder){}
		sync_cout<<"bestmove 0000"<<sync_endl;
		return res;
	}else if(legalMoves==1){
		sync_cout<<"info pv "<<p.displayUci(lastLegalMove)<<sync_endl;
		while((limits.infinite && !signals.stop) || limits.ponder){}
		sync_cout<<"bestmove "<<p.displayUci(lastLegalMove)<<sync_endl;
		return res;
	}

	//----------------------------------------------
	//	book probing
	//----------------------------------------------

	PolyglotBook pol;
	//std::cout<<"polyglot testing"<<std::endl;
	if(useOwnBook && !limits.infinite ){
		Move bookM=pol.probe(p,"book.bin",bestMoveBook);
		if(bookM.packed){
			sync_cout<<"info pv "<<p.displayUci(bookM)<<sync_endl;
			while((limits.infinite && !signals.stop) || limits.ponder){}
			sync_cout<<"bestmove "<<p.displayUci(bookM)<<sync_endl;
			return res;
		}
	}



	unsigned int selDepthBase=p.getActualState().ply;

	startTime = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
	std::vector<Move> newPV;
	unsigned int depth=1;

	Score alpha=-SCORE_INFINITE,beta =SCORE_INFINITE;
	Score delta=1600;
	selDepth=selDepthBase;

	do{

		for (rootMove& rm : rootMoves){
			rm.previousScore = rm.score;
			rm.score=-SCORE_INFINITE;
		}

		for (PVIdx = 0; PVIdx < PVSize; PVIdx++)
		{

			//sync_cout<<"PVIdx="<<PVIdx<<sync_endl;



			if (depth >= 5)
			{
				delta = 800;
				alpha = std::max((signed long long int)(rootMoves[PVIdx].previousScore) - delta,(signed long long int)-SCORE_INFINITE);
				beta  = std::min((signed long long int)(rootMoves[PVIdx].previousScore) + delta,(signed long long int) SCORE_INFINITE);
			}
			//sync_cout<<"alpha="<<alpha<<sync_endl;
			//sync_cout<<"beta="<<beta<<sync_endl;

			// reload the last PV in the transposition table
			for(unsigned int i =0; i<=PVIdx; i++){
				int n=0;
				if(/*nodeType==typeExact && */rootMoves[i].PV.size()>0){
					for (auto it= rootMoves[i].PV.begin(); it != rootMoves[i].PV.end() && mg.isMoveLegal(*it); ++it){
							/*if(!mg.isMoveLegal(*it)){
								p.display();
								sync_cout<<"move:"<<p.displayUci(*it)<<sync_endl;
								sync_cout<<"packed:"<<(*it).packed<<sync_endl;
								break;
							}*/

							TT.store(p.getActualState().key, transpositionTable::scoreToTT((n%2)?-rootMoves[i].previousScore:rootMoves[i].previousScore, n),typeExact,depth-n*ONE_PLY, (*it).packed, p.eval<false>(pawnHashTable,evalHashTable));


							//sync_cout<<"insert in TT "<<p.displayUci(*it)<<sync_endl;
							p.doMove(*it);
							n++;

					}
					for (n--;n>=0;n--){
						//sync_cout<<"undo move "<<p.displayUci(rootMoves[i].PV[n])<<sync_endl;
						p.undoMove(rootMoves[i].PV[n]);
					}
				}
			}

			unsigned int reduction=0;

			do{

				//sync_cout<<"SEARCH"<<sync_endl;
				selDepth=selDepthBase;
				newPV.clear();
				p.cleanStateInfo();
				//sync_cout<<"search line "<<PVIdx<<sync_endl;
				res=alphaBeta<search::nodeType::ROOT_NODE>(0,p,(depth-reduction)*ONE_PLY,alpha,beta,newPV);

				/*sync_cout<<"FINISHED SEARCH"<<sync_endl;
				sync_cout<<"res="<<res<<sync_endl;
				sync_cout<<p.displayUci(newPV[0])<<sync_endl;
				sync_cout<<"PVsize "<<newPV.size()<<sync_endl;*/

				if(depth!=1 && signals.stop){
					//sync_cout<<"iterative deepening Stop"<<sync_endl;
					break;
				}
				unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
				if(newPV.size()!=0 && res > alpha /*&& res < beta*/){
					std::vector<rootMove>::iterator it=std::find(rootMoves.begin(),rootMoves.end(),newPV[0]);
					if(it->firstMove==newPV[0]){
						it->PV=newPV;
						it->score=res;
						it->previousScore=res;
						it->selDepth=selDepth-selDepthBase;
						it->depth=depth-reduction;
						it->nodes=visitedNodes;
						it->time= now-startTime;
					}
					std::stable_sort(rootMoves.begin() + PVIdx, rootMoves.end());
					//sync_cout<<"stableSort OK "<<sync_endl;

				}

				if (res <= alpha)
				{
					//sync_cout<<"res<=alpha "<<sync_endl;

					//my_thread::timeMan.idLoopRequestToExtend=true;
					printPV(res,depth-reduction,selDepth-selDepthBase,alpha,beta, p, now-startTime,PVIdx,newPV,visitedNodes);
					alpha = std::max((signed long long int)(res) - delta,(signed long long int)-SCORE_INFINITE);

					TT.store(p.getActualState().key, transpositionTable::scoreToTT(rootMoves[PVIdx].previousScore, 0),typeExact,depth*ONE_PLY, (rootMoves[PVIdx].PV[0]).packed, p.eval<false>(pawnHashTable, evalHashTable));
					//sync_cout<<"new alpha "<<alpha<<sync_endl;
				}
				else if (res >= beta){
					if(oldBestMove.packed && oldBestMove!=newPV[0]){
						my_thread::timeMan.allocatedTime=my_thread::timeMan.maxSearchTime;
						//sync_cout<<"estesa ricerca="<<my_thread::timeMan.allocatedTime<<sync_endl;
					}
					//sync_cout<<"res>=beta "<<sync_endl;
					printPV(res,depth-reduction,selDepth-selDepthBase,alpha,beta, p, now-startTime,PVIdx,newPV,visitedNodes);
					beta = std::min((signed long long int)(res) + delta, (signed long long int)SCORE_INFINITE);
					if(depth>1){
						reduction=1;
					}
					//sync_cout<<"new beta "<<beta<<sync_endl;
				}else{
					break;
				}
				delta += delta / 2;


			}while(1);
			//sync_cout<<"aspiration window ok "<<sync_endl;
			if(!signals.stop){

				// Sort the PV lines searched so far and update the GUI
				std::stable_sort(rootMoves.begin(), rootMoves.begin() + PVIdx + 1);
				//sync_cout<<"stable sort ok "<<sync_endl;
/*				unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
				if (PVIdx + 1 == PVSize
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
					|| now - startTime > 3000
#endif
				)*/{
					//sync_cout<<"print pv "<<sync_endl;
					printAllPV(p, PVSize);

				}
			}
		}
		//-----------------------
		//	single good move at root
		//-----------------------
		if (depth >= 12
			&& !signals.stop
			&&  PVSize == 1
			&&  res > SCORE_MATED_IN_MAX_PLY)
		{

			Score rBeta = res - 20000;
			p.getActualState().excludedMove=newPV[0];
			p.getActualState().skipNullMove=true;
			std::vector<Move> locChildPV;
			Score temp = alphaBeta<search::nodeType::ALL_NODE>(0,p,(depth-3)*ONE_PLY,rBeta-1,rBeta,locChildPV);
			p.getActualState().skipNullMove=false;
			p.getActualState().excludedMove.packed=0;

			if(temp < rBeta){
				my_thread::timeMan.singularRootMoveCount++;
			}
		}

		if(oldBestMove.packed && oldBestMove!=newPV[0]){
			my_thread::timeMan.allocatedTime=my_thread::timeMan.maxSearchTime;
			//sync_cout<<"estesa ricerca="<<my_thread::timeMan.allocatedTime<<sync_endl;
		}
		oldBestMove=newPV[0];

		my_thread::timeMan.depth=depth;
		unsigned long time=my_thread::timeMan.minSearchTime;
		my_thread::timeMan.allocatedTime=std::max(time,(unsigned long int)(my_thread::timeMan.allocatedTime*0.90));
		//sync_cout<<"nuovo tempo allocato="<<my_thread::timeMan.allocatedTime<<sync_endl;

		my_thread::timeMan.idLoopIterationFinished=true;


		depth+=1;

	}while(depth<=(limits.depth? limits.depth:100) && !signals.stop);

	//sync_cout<<"print final bestMove "<<sync_endl;

	while(limits.ponder){
	}

	unsigned int bestMoveLine=0;
	if(limitStrength){
		double lambda=(eloStrenght-1000.0)*(0.8/2000)+0.2;

		std::mt19937_64 rnd;
		std::exponential_distribution<> uint_dist(lambda);
		unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
		rnd.seed(now);
		double dres = uint_dist(rnd);

		unsigned int pos=dres;
		bestMoveLine =std::min(pos,PVSize-1);
	}
	//sync_cout<<"bestMove index "<<bestMoveLine<<sync_endl;


	sync_cout<<"bestmove "<<p.displayUci(rootMoves[bestMoveLine].PV[0]);
	if(rootMoves[0].PV.size()>1)
	{
		std::cout<<" ponder "<<p.displayUci(rootMoves[bestMoveLine].PV[1]);
	}
	std::cout<<sync_endl;
#ifdef PRINT_STATISTICS
	Statistics::instance().printNodeTypeStat();
#endif

	return res;



}

template<search::nodeType type> Score search::alphaBeta(unsigned int ply,Position & pos,int depth,Score alpha,Score beta,std::vector<Move>& PV){

	//bool verbose=false;
#ifdef PRINT_STATISTICS
	Score originalAlpha=alpha;
#endif
	assert(alpha <beta);
	assert(alpha>=-SCORE_INFINITE);
	assert(beta<=SCORE_INFINITE);
	assert(depth>=ONE_PLY);
	assert(PV.size()==0);
	/*if(visitedNodes>709000 && visitedNodes<710000){
		sync_cout<<visitedNodes<<" AB "<<"ply:"<<ply<<" depth: "<<depth<<" alpha:"<<alpha<<" beta:"<<beta<<" "<<pos.displayFen()<<sync_endl;
	}*/
	//sync_cout<<"AB "<<"ply:"<<ply<<" depth: "<<depth<<" alpha:"<<alpha<<" beta:"<<beta<<" "<<pos.displayFen()<<sync_endl;
	//pos.display();
	/*if(pos.displayFen()=="rn1qkb1r/ppp2ppp/4bn2/1B6/8/5N2/PPPP1PPP/RNBQK2R b KQkq - 1 5"){
		sync_cout<<"eccomi"<<sync_endl;
		verbose=true;
		pos.display();
	}*/
	//if(verbose){sync_cout<<"eccomi"<<sync_endl;}
	visitedNodes++;
	//Position::state & actualState=pos.getActualState();
	const bool PVnode=(type==search::nodeType::PV_NODE || type==search::nodeType::ROOT_NODE);
	const bool inCheck = pos.getActualState().checkers;
	Move threatMove;
	threatMove=0;

	if( showLine && depth <=ONE_PLY){
		showLine=false;
		sync_cout<<"info currline";
		for (int i =1; i<= pos.getStateIndex()/2;i++){ // show only half of
			std::cout<<" "<<pos.displayUci(pos.getState(i).currentMove);

		}
		std::cout<<sync_endl;
	}


	const search::nodeType childNodesType=
			type==search::nodeType::ALL_NODE?
					search::nodeType::CUT_NODE:
					type==search::nodeType::CUT_NODE?search::nodeType::ALL_NODE:
							search::nodeType::PV_NODE;

	if(type !=search::nodeType::ROOT_NODE){
		if(pos.isDraw(PVnode) || signals.stop){
			//if(signals.stop){sync_cout<<"alpha beta initial Stop"<<sync_endl;}
			return 0;
		}

		//---------------------------------------
		//	MATE DISTANCE PRUNING
		//---------------------------------------
		alpha = std::max(matedIn(ply), alpha);
		beta = std::min(mateIn(ply+1), beta);
		if (alpha >= beta){
#ifdef PRINT_STATISTICS
			Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
#endif
			return alpha;
		}

	}


	Move excludedMove=pos.getActualState().excludedMove;

	U64 posKey=excludedMove.packed?pos.getExclusionKey() :pos.getKey();
	ttEntry* tte = TT.probe(posKey);
	Move ttMove;
	ttMove=tte!=nullptr ? tte->getPackedMove() : 0;
	Score ttValue = tte!=nullptr ? transpositionTable::scoreFromTT(tte->getValue(),ply) : SCORE_NONE;

	if (type!=search::nodeType::ROOT_NODE &&
			tte!=nullptr
			&& tte->getDepth() >= depth
		    && ttValue != SCORE_NONE // Only in case of TT access race
		    //&& (	PVnode ?  tte->getType() == typeExact
		    // TODO vedere se nei PV node in cui ho un beta cutoff o un alpha cutoff ritornare il valore del TT
		    && (	PVnode ?  false
		            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact)
		                              : (tte->getType() ==  typeScoreLowerThanAlpha || tte->getType() == typeExact)))
	{
		TT.refresh(tte);

		if (ttValue >= beta &&
			ttMove.packed &&
			!pos.isCaptureMoveOrPromotion(ttMove) &&
			!inCheck)
		{
			if(pos.getActualState().killers[0] != ttMove)
			{
				pos.getActualState().killers[1] = pos.getActualState().killers[0];
				pos.getActualState().killers[0] = ttMove;
			}
		}


		Movegen mg(pos,history,ttMove);
		if(ttMove.packed && mg.isMoveLegal(ttMove)){
			PV.clear();
			PV.push_back(ttMove);
		}
#ifdef PRINT_STATISTICS
		if(ttValue>=beta){
			Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
		}
		else if(ttValue<=alpha){
			Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
		}
		else{
			Statistics::instance().gatherNodeTypeStat(type,PV_NODE);
		}
#endif
		return ttValue;
	}

	Score staticEval;
	Score eval;
	if(inCheck){
		staticEval=pos.eval<false>(pawnHashTable, evalHashTable);
		pos.getActualState().staticEval=SCORE_NONE;
		eval=staticEval;

#ifdef DEBUG_EVAL_SIMMETRY
		Position ppp;
		ppp.setupFromFen(pos.getSymmetricFen());
		Score test=ppp.eval<false>(pawnHashTable,evalHashTable);
		if(test!=eval){
			pos.display();
			while(1);
		}
#endif
	}
	else{
		if(tte!=nullptr)
		{
			staticEval=tte->getStaticValue();
			pos.getActualState().staticEval=staticEval;
			assert(staticEval<SCORE_INFINITE);
			assert(staticEval>-SCORE_INFINITE);
			eval=staticEval;
			if (ttValue != SCORE_NONE){
				if (
						((tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact) && (ttValue > eval) )
						|| ((tte->getType() == typeScoreLowerThanAlpha || tte->getType() == typeExact ) && (ttValue < eval) )
					)
				{
					eval = ttValue;
				}
			}

		}
		else
		{
			staticEval=pos.eval<false>(pawnHashTable, evalHashTable);
			eval=staticEval;
			pos.getActualState().staticEval=staticEval;
#ifdef DEBUG_EVAL_SIMMETRY
			Position ppp;
			ppp.setupFromFen(pos.getSymmetricFen());
			Score test=ppp.eval<false>(pawnHashTable,evalHashTable);
			if(test!=eval){
				pos.display();
				while(1);
			}
#endif
		}

	}

	// update gains
	if (pos.getStateIndex()>=2 &&
			pos.getActualState().capturedPiece==Position::empty
			&&  pos.getActualState().staticEval != SCORE_NONE
		&& pos.getPreviousState().staticEval != SCORE_NONE
		&& pos.getActualState().currentMove.packed != 0)
	{

		gains.update(pos.squares[pos.getActualState().currentMove.to],(tSquare)pos.getActualState().currentMove.to,-pos.getPreviousState().staticEval-eval);
	}

	//------------------------
	// razoring
	//------------------------
	// at very low deep and with an evaluation well below alpha, if a qsearch don't raise the evaluation then prune the node.
	//------------------------
	if (!PVnode
		&& !inCheck
		&&  depth < 4 * ONE_PLY
		&&  (eval + razorMargin(depth) <= alpha)
		&&  alpha >= -SCORE_INFINITE+razorMargin(depth)
		//&&  abs(alpha) < SCORE_MATE_IN_MAX_PLY // implicito nell riga precedente
		&&  ((!ttMove.packed ) || type==ALL_NODE)
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		&& !((pos.getActualState().nextMove && (pos.bitBoard[Position::blackPawns] & RANKMASK[A2])) || (!pos.getActualState().nextMove && (pos.bitBoard[Position::whitePawns] & RANKMASK[A7]) ) )
	)
	{
		Score ralpha = alpha - razorMargin(depth);
		assert(ralpha>=-SCORE_INFINITE);
		std::vector<Move> childPV;
		Score v = qsearch<childNodesType>(ply,pos,0, ralpha, ralpha+1, childPV);
		if (v <= ralpha)
		{
			return v;
		}
	}

	//---------------------------
	//	 STATIC NULL MOVE PRUNING
	//---------------------------
	//	at very low deep and with an evaluation well above beta, bet that we can found a move with a result above beta
	//---------------------------
	if (!PVnode
		&& !inCheck
		&& !pos.getActualState().skipNullMove
		&&  depth < 4 * ONE_PLY
		&& eval >-SCORE_INFINITE + futility[depth>>ONE_PLY_SHIFT]
		&&  eval - futility[depth>>ONE_PLY_SHIFT] >= beta
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		//&&  abs(eval) < SCORE_KNOWN_WIN
		&&  ((pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[2]>= Position::pieceValue[Position::whiteKnights][0]) || (!pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[0]>= Position::pieceValue[Position::whiteKnights][0])))
	{
		assert((eval -futility[depth>>ONE_PLY_SHIFT] >-SCORE_INFINITE));
#ifdef PRINT_STATISTICS
		Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
#endif
		return eval - futility[depth>>ONE_PLY_SHIFT];
	}


	//---------------------------
	//	 NULL MOVE PRUNING
	//---------------------------
	// if the evaluation is above beta and after passing the move the result of a search is still above beta we bet there will be a beta cutoff
	// this search let us know about threat move by the opponent.
	//---------------------------

	if(!PVnode
		&& !inCheck
		&& depth>=ONE_PLY
		&& eval>=beta
		&& !pos.getActualState().skipNullMove
		&&  abs(beta) < SCORE_MATE_IN_MAX_PLY
		&&((pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[2]>= Position::pieceValue[Position::whiteKnights][0]) || (!pos.getActualState().nextMove && pos.getActualState().nonPawnMaterial[0]>= Position::pieceValue[Position::whiteKnights][0]))
	){
		// Null move dynamic reduction based on depth
		int red = 3 * ONE_PLY + depth / 4;

		// Null move dynamic reduction based on value
		if (eval > -SCORE_INFINITE+10000 && eval - 10000 > beta){
			red += ONE_PLY;
		}
		pos.doNullMove();
		U64 nullKey= pos.getActualState().key;


		std::vector<Move> childPV;
		Score nullVal;
		if(depth-red<ONE_PLY ){
			nullVal = -qsearch<childNodesType>(ply+1,pos,0,-beta,-beta+1,childPV);
		}else
		{
			nullVal = -alphaBeta<childNodesType>(ply+1,pos, depth - red, -beta, -beta+1, childPV);
		}
		pos.undoNullMove();

		if (nullVal >= beta)
		{
			{
				// Do not return unproven mate scores
				if (nullVal >= SCORE_MATE_IN_MAX_PLY){
					nullVal = beta;
				}

				if (depth < 12 * ONE_PLY){
#ifdef PRINT_STATISTICS
					Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
#endif
					return nullVal;
				}

				childPV.clear();
				// Do verification search at high depths
				pos.getActualState().skipNullMove=true;
				assert(depth - red>=ONE_PLY);
				Score val;
				if(depth-red<ONE_PLY){
					val = qsearch<childNodesType>(ply,pos, 0, beta-1, beta, childPV);
				}
				else{
					val = alphaBeta<childNodesType>(ply,pos, depth - red, beta-1, beta, childPV);
				}
				pos.getActualState().skipNullMove=false;
				if (val >= beta){
#ifdef PRINT_STATISTICS
					Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
#endif
					return nullVal;
				}
			}

		}
		else
		{
			ttEntry * tte=TT.probe(nullKey);
			threatMove=tte!=nullptr? tte->getPackedMove(): 0;
		}

	}

	//------------------------
	//	PROB CUT
	//------------------------
	//	at high depth we try the capture moves. if a reduced search of this moves gives us a result above beta we bet we can found with a regular search a move exceeding beta
	//------------------------

	if(!PVnode &&
		!inCheck &&
		depth>=5*ONE_PLY &&
		!pos.getActualState().skipNullMove &&
		abs(beta)<SCORE_KNOWN_WIN
		//eval> beta-40000
		//&& abs(beta)<SCORE_MATE_IN_MAX_PLY
	){
		Score s;
		Score rBeta=beta+8000;
		int rDepth=depth -ONE_PLY- 3*ONE_PLY;
		Movegen mg(pos,history,ttMove);
		mg.setupProbCutSearch(pos.getActualState().capturedPiece);

		Move m;
		while((m=mg.getNextMove()).packed){
			pos.doMove(m);
			std::vector<Move> childPV;
			assert(rDepth>=ONE_PLY);
			s=-alphaBeta<childNodesType>(ply+1,pos,rDepth,-rBeta,-rBeta+1,childPV);
			pos.undoMove(m);
			if(s>=rBeta){
#ifdef PRINT_STATISTICS
				Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
#endif
				return s;
			}

		}
	}

	//------------------------
	//	IID
	//------------------------
	if(depth >= (PVnode ? 5 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove.packed == 0
		&& (PVnode || staticEval+10000>= beta))
	{
		int d = depth - 2 * ONE_PLY - (PVnode ? 0 : depth / 4);

		bool skipBackup=pos.getActualState().skipNullMove;
		pos.getActualState().skipNullMove=true;

		std::vector<Move> childPV;
		const search::nodeType iidType=type;
		assert(d>=ONE_PLY);
		alphaBeta<iidType>(ply,pos, d, alpha, beta, childPV);

		pos.getActualState().skipNullMove=skipBackup;

		tte = TT.probe(posKey);
		ttMove= tte!=nullptr ? tte->getPackedMove():0;
	}




	Score bestScore=-SCORE_INFINITE;

	Move bestMove;
	bestMove=0;
	Move m;
	Movegen mg(pos,history,ttMove);
	unsigned int moveNumber=0;
	unsigned int quietMoveCount =0;
	Move quietMoveList[64];

	bool singularExtensionNode=
		type!=search::nodeType::ROOT_NODE
		&& depth >= (PVnode ? 6 * ONE_PLY : 8 * ONE_PLY)
		&& ttMove.packed != 0
		&& !excludedMove.packed // Recursive singular search is not allowed
		&& tte!=nullptr
		&& (tte->getType() ==  typeScoreHigherThanBeta || tte->getType() == typeExact)
		&&  tte->getDepth()>= depth - 3 * ONE_PLY;

	//sync_cout<<"iterating moves"<<sync_endl;
	while (bestScore <beta  && (m=mg.getNextMove()).packed) {

#ifdef PRINT_STATISTICS
		bool testedAll=false;
		bool testedCut=false;
#endif
		/*if(verbose){
			sync_cout<<"move "<<pos.displayUci(m)<<sync_endl;
		}*/

		assert(m.packed);
		if(m== excludedMove){
			continue;
		}


		// search only the moves in the search list
		if(type==search::nodeType::ROOT_NODE && !std::count(rootMoves.begin()+PVIdx,rootMoves.end(),m)){
			//sync_cout<<"avoid move "<<pos.displayUci(m)<<sync_endl;
			continue;
		}

		moveNumber++;


		bool captureOrPromotion =pos.isCaptureMoveOrPromotion(m);
		if(!captureOrPromotion && quietMoveCount < 64){
			quietMoveList[quietMoveCount++]=m;
		}

		bool moveGivesCheck=pos.moveGivesCheck(m);
		bool isDangerous=moveGivesCheck || pos.isCastleMove(m) || pos.isPassedPawnMove(m);

		int ext=0;
		if(PVnode && isDangerous){
			ext = ONE_PLY;
		}else if(moveGivesCheck && pos.seeSign(m) >= 0){
			ext = ONE_PLY / 2;
		}

		//------------------------------
		//	SINGULAR EXTENSION NODE
		//------------------------------
		if(singularExtensionNode
			&& !ext
			&&  m == ttMove
			&&  abs(ttValue) < SCORE_KNOWN_WIN
		)
		{

			Score rBeta = ttValue - int(depth*10);
			pos.getActualState().excludedMove=m;
			bool backup=pos.getActualState().skipNullMove;
			pos.getActualState().skipNullMove=true;
			std::vector<Move> locChildPV;
			Score temp = alphaBeta<childNodesType>(ply,pos,depth/2,rBeta-1,rBeta,locChildPV);
			pos.getActualState().skipNullMove=backup;
			pos.getActualState().excludedMove.packed=0;

			if(temp < rBeta){
				ext = ONE_PLY;
		    }
		}

		int newDepth= depth-ONE_PLY+ext;


		//---------------------------------------
		//	FUTILITY PRUNING
		//---------------------------------------
		if(!PVnode
			&& !captureOrPromotion
			&& !inCheck
			&& m != ttMove
			&& !isDangerous
			&& bestScore>SCORE_MATED_IN_MAX_PLY
		){
			assert(moveNumber>1);

			if(newDepth < 11*ONE_PLY
				&& moveNumber >= FutilityMoveCounts[newDepth>>ONE_PLY_SHIFT]
				&& (!threatMove.packed)
				)
			{
				assert((newDepth>>ONE_PLY_SHIFT)<11);
				continue;
			}


			if(newDepth<7*ONE_PLY ){
				assert((newDepth>>ONE_PLY_SHIFT)<=6);
				Score localEval= eval +625*newDepth+ gains.getValue(pos.squares[m.from],(tSquare)m.to);
				if(localEval<beta && history.getValue(pos.squares[m.from],(tSquare)m.to)>=0){

#ifdef PRINT_STATISTICS
				//pos.display();
				//sync_cout<<"move: "<<pos.displayUci(m)<<sync_endl;
			if(type==ALL_NODE){
				//sync_cout<<"ALL"<<sync_endl;
				testedAll=true;
				Statistics::instance().testedAllPruning[newDepth]++;

			}
			else{
				//sync_cout<<"CUT"<<sync_endl;
				testedCut=true;
				Statistics::instance().testedCutPruning[newDepth]++;
			}
#endif
					bestScore = std::max(bestScore, localEval);
					assert((newDepth>>ONE_PLY_SHIFT)<7);
					continue;
				}
			}

			if(//newDepth < 60 * ONE_PLY &&
				pos.seeSign(m) < 0)
			{

				continue;
			}



		}


		if(type==ROOT_NODE){
			unsigned long elapsed=std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count()-startTime;
			if(
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
				elapsed>3000 &&
#endif
				!signals.stop
				){
				sync_cout<<"info currmovenumber "<<moveNumber<<" currmove "<<pos.displayUci(m)<<" nodes "<<visitedNodes<<
#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
						" time "<<elapsed <<
#endif
						sync_endl;
			}
		}

		//sync_cout<<"MOVE:"<<pos.displayUci(m)<<sync_endl;
		pos.doMove(m);

		Score val;
		std::vector<Move> childPV;
		if(PVnode){
			if(moveNumber==1){
#ifdef DEBUG1
				if(type==search::nodeType::ROOT_NODE){
					sync_cout<<"FIRST alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<sync_endl;
				}
#endif

				if(newDepth<ONE_PLY){
					val=-qsearch<search::nodeType::PV_NODE>(ply+1,pos,newDepth,-beta,-alpha,childPV);
				}else{
					val=-alphaBeta<search::nodeType::PV_NODE>(ply+1,pos,newDepth,-beta,-alpha,childPV);
				}
#ifdef PRINT_PV_CHANGES
				sync_cout<<"FIRST ply "<<ply<<" alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<" res "<<val/10000.0<<" "<<pos.displayUci(m)<<sync_endl;
#endif
			}
			else{

				//------------------------------
				//	LMR
				//------------------------------
				bool doFullDepthSearch=true;
				if(depth>=3*ONE_PLY
					&& !captureOrPromotion
					&& !isDangerous
					&&  m != ttMove
					&&  m != mg.killerMoves[0]
					&&  m != mg.killerMoves[1]
				)
				{
					int reduction = PVreduction[std::min(depth,32*ONE_PLY-1)][std::min(moveNumber,(unsigned int)63)];
					int d = std::max(newDepth - reduction, ONE_PLY);

					if(reduction!=0){
#ifdef DEBUG1
					if(type==search::nodeType::ROOT_NODE){
						sync_cout<<"LMR alpha "<<(alpha)/10000.0<<" beta "<<(alpha+1)/10000.0<<" red "<<(float)(reduction)/ONE_PLY<<sync_endl;
					}
#endif
						val=-alphaBeta<search::nodeType::CUT_NODE>(ply+1,pos,d,-alpha-1,-alpha,childPV);
						if(val<=alpha){
							doFullDepthSearch=false;
						}
					}
				}


				if(doFullDepthSearch){

					childPV.clear();
#ifdef DEBUG1
					if(type==search::nodeType::ROOT_NODE){
						sync_cout<<"OTHER alpha "<<(alpha)/10000.0<<" beta "<<(alpha+1)/10000.0<<sync_endl;
					}
#endif
					if(newDepth<ONE_PLY){
						val=-qsearch<search::nodeType::CUT_NODE>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}else{
						val=-alphaBeta<search::nodeType::CUT_NODE>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}

					if(val>alpha && val < beta ){
#ifdef DEBUG1
						if(type==ROOT_NODE){
							sync_cout<<"info currmove "<<pos.displayUci(m)<<" val "<<val/10000.0<<" nodes "<<visitedNodes<< sync_endl;
						}
#endif
						childPV.clear();
#ifdef DEBUG1
						if(type==search::nodeType::ROOT_NODE){
							sync_cout<<"RESEARCH alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<sync_endl;
						}
#endif
						if(newDepth<ONE_PLY){
							val=-qsearch<search::nodeType::PV_NODE>(ply+1,pos,newDepth,-beta,-alpha,childPV);
						}
						else{
							val=-alphaBeta<search::nodeType::PV_NODE>(ply+1,pos,newDepth,-beta,-alpha,childPV);
						}
#ifdef PRINT_PV_CHANGES
						sync_cout<<"OTHER ply "<<ply<<" alpha "<<alpha/10000.0<<" beta "<<beta/10000.0<<" res "<<val/10000.0<<" "<<pos.displayUci(m)<<sync_endl;
#endif
					}
				}

			}
		}
		else{

			//------------------------------
			//	LMR
			//------------------------------
			bool doFullDepthSearch=true;
			if(depth>=3*ONE_PLY
				&& !captureOrPromotion
				&& !isDangerous
				&&  m != ttMove
				&&  m != mg.killerMoves[0]
				&&  m != mg.killerMoves[1]
			)
			{
				int reduction = nonPVreduction[std::min(depth,32*ONE_PLY-1)][std::min(moveNumber,(unsigned int)63)];
				int d = std::max(newDepth - reduction, ONE_PLY);

				if(reduction!=0){
					childPV.clear();
					val=-alphaBeta<childNodesType>(ply+1,pos,d,-alpha-1,-alpha,childPV);
					if(val<=alpha){
						doFullDepthSearch=false;
					}
				}
			}




			if(doFullDepthSearch){
				childPV.clear();

				if(moveNumber<5){
					if(newDepth<ONE_PLY){
						/*if(verbose){
							sync_cout<<"qsearch depth "<<newDepth<<sync_endl;
						}*/
						val=-qsearch<childNodesType>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}else{
						/*if(verbose){
							sync_cout<<"search depth "<<newDepth<<sync_endl;
						}*/
						val=-alphaBeta<childNodesType>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}
				}
				else{
					if(newDepth<ONE_PLY){
						/*if(verbose){
							sync_cout<<"qsearch depth "<<newDepth<<sync_endl;
						}*/
						val=-qsearch<search::nodeType::CUT_NODE>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}else{
						/*if(verbose){
							sync_cout<<"search depth "<<newDepth<<sync_endl;
						}*/
						val=-alphaBeta<search::nodeType::CUT_NODE>(ply+1,pos,newDepth,-alpha-1,-alpha,childPV);
					}

				}

			}
		}



		pos.undoMove(m);



#ifdef PRINT_STATISTICS

				/*if(testedAll==true){
					sync_cout<<"val ?? alpha "<<val<<"<="<<alpha<<sync_endl;
				}
				if(testedCut==true){
					sync_cout<<"val ?? alpha: "<<val<<"<="<<alpha<<sync_endl;
				}*/


				if(val>=beta){

				if(testedAll==true){

					//sync_cout<<"eval:"<<eval<<" val:"<<val<<sync_endl;
					//sync_cout<<"val<=alpha: "<<val<<"<="<<alpha<<std::endl<<sync_endl;
					Statistics::instance().correctAllPruning[newDepth]++;
				}
				if(testedCut==true){
					//sync_cout<<"eval:"<<eval<<" val:"<<val<<sync_endl;
					//sync_cout<<"val<=alpha: "<<val<<"<="<<alpha<<std::endl<<sync_endl;
					Statistics::instance().correctCutPruning[newDepth]++;
				}
				}
#endif

		if(val>bestScore){
			bestScore=val;

			if(val>alpha){
				bestMove=m;
				if(PVnode)
				{
					alpha =val;
				}
				if(type ==search::nodeType::ROOT_NODE|| (PVnode &&!signals.stop)){

					PV.clear();
					PV.push_back(bestMove);
					std::copy (childPV.begin(),childPV.end(),back_inserter(PV));
				}
			}



		}
	}


	// draw

	if(!moveNumber){
		if( excludedMove.packed){
#ifdef PRINT_STATISTICS
			Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
#endif
			return alpha;
		}else if(!inCheck){
			bestScore=0;
		}
		else{
			bestScore=matedIn(ply);
		}
	}

	if (bestScore == -SCORE_INFINITE)
		bestScore = alpha;


	if(!signals.stop){
	//Statistics::instance().gatherNodeTypeStat(type,bestScore >= beta?CUT_NODE:PVnode && bestMove.packed? PV_NODE:ALL_NODE );
	TT.store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta  ? typeScoreHigherThanBeta :
					(PVnode && bestMove.packed) ? typeExact : typeScoreLowerThanAlpha,
							depth, bestMove.packed, staticEval);
	}


	// save killer move & update history
	if (bestScore >= beta &&
			// TODO provare a fare solamente pos.isCaptureMove
		!pos.isCaptureMoveOrPromotion(bestMove) &&
		!inCheck)
	{
		if(pos.getActualState().killers[0] != bestMove)
		{
			pos.getActualState().killers[1] = pos.getActualState().killers[0];
			pos.getActualState().killers[0] = bestMove;
		}


		// update history
		// todo controllare se fare +=depth^2 e -=(depth^2)/(numero di mosse quiet) per avere media nulla
		// todo controllare se usare depth o qualche depth scalata
		Score bonus = Score(depth * depth);
		history.update(pos.squares[bestMove.from],(tSquare) bestMove.to, bonus);
		if(quietMoveCount>1){
			for (int i = 0; i < quietMoveCount - 1; i++){
				Move m;
				m= quietMoveList[i];
				history.update(pos.squares[m.from],(tSquare) m.to, -bonus);
			}
		}
	}
#ifdef PRINT_STATISTICS

	if(bestScore>=beta){
		Statistics::instance().cutNodeOrderingArray[moveNumber]++;
		Statistics::instance().cutNodeOrderingSum+=moveNumber;
		Statistics::instance().cutNodeOrderingCounter++;
		if(Statistics::instance().worstCutNodeOrdering<moveNumber){
			Statistics::instance().worstCutNodeOrdering=moveNumber;
		}
	}else if(PVnode && bestMove.packed){
		Statistics::instance().allNodeOrderingSum+=moveNumber;
		Statistics::instance().allNodeOrderingCounter++;
	}else{
		Statistics::instance().pvNodeOrderingSum+=moveNumber;
		Statistics::instance().pvNodeOrderingCounter++;
	}


	if(bestScore>=beta){

		Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
	}else if(PVnode && bestMove.packed){

		Statistics::instance().gatherNodeTypeStat(type,PV_NODE);
	}else{
		Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
	}
#endif
	return bestScore;

}


template<search::nodeType type> Score search::qsearch(unsigned int ply,Position & pos,int depth,Score alpha,Score beta,std::vector<Move>& PV){

#ifdef PRINT_STATISTICS
	Score originalAlpha=alpha;
#endif
	assert(ply>0);
	assert(depth<ONE_PLY);
	assert(alpha<beta);
	assert(PV.size()==0);
	assert(beta<=SCORE_INFINITE);
	assert(alpha>=-SCORE_INFINITE);



	/*if(visitedNodes>599000 && visitedNodes<800000){
		sync_cout<<"Q ply:"<<ply<<" depth: "<<depth<<" alpha:"<<alpha<<" beta:"<<beta<<" "<<pos.displayFen()<<sync_endl;
	}*/
	//sync_cout<<"Q ply:"<<ply<<" depth: "<<depth<<" alpha:"<<alpha<<" beta:"<<beta<<" "<<pos.displayFen()<<sync_endl;
	//pos.display();

	//Position::state & actualState=pos.getActualState();
	const bool PVnode=(type==search::nodeType::PV_NODE);
	assert(PVnode || alpha+1==beta);
	bool inCheck=pos.getActualState().checkers;


	if(pos.getActualState().ply>selDepth){
		selDepth=pos.getActualState().ply;
	}
	visitedNodes++;

	if(pos.isDraw(PVnode) || signals.stop){
#ifdef PRINT_STATISTICS
		Statistics::instance().gatherNodeTypeStat(type,PV_NODE);
#endif
		//sync_cout<<"DRAW"<<sync_endl;
		return 0;
	}



	//----------------------------
	//	next node type
	//----------------------------
	const search::nodeType childNodesType=
		type==search::nodeType::ALL_NODE?
			search::nodeType::CUT_NODE:
			type==search::nodeType::CUT_NODE?search::nodeType::ALL_NODE:
				search::nodeType::PV_NODE;



	U64 posKey=pos.getActualState().key;
	ttEntry* tte = TT.probe(posKey);
	Move ttMove;
	ttMove=tte ? tte->getPackedMove() : 0;
	Movegen mg(pos,history,ttMove);
	int TTdepth=mg.setupQuiescentSearch(pos.getActualState().checkers,depth);
	Score ttValue = tte ? transpositionTable::scoreFromTT(tte->getValue(),ply) : SCORE_NONE;

	if (tte
		&& tte->getDepth() >= TTdepth
	    && ttValue != SCORE_NONE // Only in case of TT access race
	    && (	PVnode ?  false/*tte->getType() == typeExact*/
	            : ttValue >= beta ? (tte->getType() ==  typeScoreHigherThanBeta|| tte->getType() == typeExact)
	                              : (tte->getType() ==  typeScoreLowerThanAlpha|| tte->getType() == typeExact)))
	{
		TT.refresh(tte);

		/*if(ttMove.packed && Movegen::isMoveLegal(ttMove)){
			PV.clear();
			PV.push_back(ttMove);
		}*/
#ifdef PRINT_STATISTICS
		if(ttValue>=beta){
			Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
		}
		else if(ttValue<=alpha){
			Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
		}
		else{
			Statistics::instance().gatherNodeTypeStat(type,PV_NODE);
		}
#endif
		//sync_cout<<"TT RETURN:"<<ttValue<<sync_endl;
		return ttValue;
	}

	ttType TTtype=typeScoreLowerThanAlpha;

	//----------------------------
	//	stand pat score
	//----------------------------
	Score staticEval;
	Score bestScore;
	if(inCheck){
		staticEval=pos.eval<false>(pawnHashTable, evalHashTable);
#ifdef DEBUG_EVAL_SIMMETRY
		Position ppp;
		ppp.setupFromFen(pos.getSymmetricFen());
		Score test=ppp.eval<false>(pawnHashTable,evalHashTable);
		if(test!=staticEval){
			pos.display();
			while(1);
		}
#endif
		bestScore=-SCORE_INFINITE;
	}
	else{
		if(tte)
		{

			staticEval=tte->getStaticValue();
		}
		else
		{
			staticEval=pos.eval<false>(pawnHashTable, evalHashTable);
#ifdef DEBUG_EVAL_SIMMETRY
			Position ppp;
			ppp.setupFromFen(pos.getSymmetricFen());
			Score test=ppp.eval<false>(pawnHashTable,evalHashTable);
			if(test!=staticEval){
				pos.display();

				while(1);
			}
#endif
		}
		bestScore=staticEval;

	}


	if(bestScore>alpha){

		alpha=bestScore;
		// TODO testare se la riga TTtype=typeExact; ha senso
		TTtype=typeExact;
		if(bestScore>=beta){
			//sync_cout<<"STANDPAT RETURN:"<<bestScore<<sync_endl;
			return bestScore;
		}
	}
	// todo trovare un valore buono per il futility
	Score futilityBase=bestScore+5000;

	//----------------------------
	//	try the captures
	//----------------------------
	unsigned int moveNumber=0;
	Move m;
	Move bestMove;
	bestMove=0;
	while (bestScore <beta  &&  (m=mg.getNextMove()).packed) {
		assert(alpha<beta);
		assert(beta<=SCORE_INFINITE);
		assert(alpha>=-SCORE_INFINITE);
		moveNumber++;

		assert(m.packed);

		if(depth<-7*ONE_PLY && !inCheck){
			if(pos.getActualState().currentMove.to!= m.to){
				continue;
			}
		}

		//----------------------------
		//	futility pruning (delta pruning)
		//----------------------------
		if(!PVnode &&
			!inCheck &&
			!pos.moveGivesCheck(m) &&
			m != ttMove &&
			m.flags != Move::fpromotion &&
			!pos.isPassedPawnMove(m)
		){
			Score futilityValue=futilityBase
                    + Position::pieceValue[pos.squares[m.to]%Position::separationBitmap][1]
                    + (m.flags == Move::fenpassant ? Position::pieceValue[Position::whitePawns][1] : 0);

			if (futilityValue < beta)
			{

				bestScore = std::max(bestScore, futilityValue);
				continue;
			}
			if (futilityBase < beta && pos.seeSign(m)<=0)
			{
				bestScore = std::max(bestScore, futilityBase);
				continue;
			}

		}


		//----------------------------
		//	don't check moves with negative see
		//----------------------------

		// TODO controllare se conviene fare o non fare la condizione type != search::nodeType::PV_NODE
		// TODO testare se aggiungere o no !movegivesCheck() &&
		if(!PVnode &&
				!inCheck &&
				m.flags != Move::fpromotion &&
				m != ttMove &&
				pos.seeSign(m)<0){
			continue;
		}

		//sync_cout<<"MOVE:"<<pos.displayUci(m)<<sync_endl;
		pos.doMove(m);
		Score val;
		std::vector<Move> childPV;
		val=-qsearch<childNodesType>(ply+1,pos,depth-ONE_PLY,-beta,-alpha,childPV);

		pos.undoMove(m);


		if(val>bestScore){
			bestScore=val;
			if(val>alpha){
				bestMove=m;
				TTtype=typeExact;
				alpha =val;
				if(PVnode &&!signals.stop){
					PV.clear();
					PV.push_back(m);
					std::copy (childPV.begin(),childPV.end(),back_inserter(PV));
				}
			}
		}

	}

	if (bestScore >= beta &&
			// TODO provare a fare solamente pos.isCaptureMove
		!pos.isCaptureMoveOrPromotion(bestMove) &&
		!inCheck)
	{
		//sync_cout<<pos.displayUci(bestMove)<<sync_endl;
		if(pos.getActualState().killers[0] != bestMove)
		{
			pos.getActualState().killers[1] = pos.getActualState().killers[0];
			pos.getActualState().killers[0] = bestMove;
		}



	}



	if(bestScore == -SCORE_INFINITE/* && inCheck*/){
		assert(inCheck);
		//sync_cout<<"MATED:"<<matedIn(ply)<<sync_endl;
		return matedIn(ply);
	}

	assert(bestScore != -SCORE_INFINITE);
	/*
	if (bestScore == -SCORE_INFINITE){
		bestScore = alpha;
	}*/

	if(!signals.stop){
		TT.store(posKey, transpositionTable::scoreToTT(bestScore, ply),
			bestScore >= beta ? typeScoreHigherThanBeta : TTtype,
			TTdepth, bestMove.packed, staticEval);
	}
#ifdef PRINT_STATISTICS

	if(bestScore>=beta){
		Statistics::instance().cutNodeOrderingArray[moveNumber]++;
		if(moveNumber){

			Statistics::instance().cutNodeOrderingSum+=moveNumber;
			Statistics::instance().cutNodeOrderingCounter++;
			if(Statistics::instance().worstCutNodeOrdering<moveNumber){
				Statistics::instance().worstCutNodeOrdering=moveNumber;
			}
		}
	}else if(PVnode && bestMove.packed){
		Statistics::instance().allNodeOrderingSum+=moveNumber;
		Statistics::instance().allNodeOrderingCounter++;
	}else{
		Statistics::instance().pvNodeOrderingSum+=moveNumber;
		Statistics::instance().pvNodeOrderingCounter++;
	}

	if(bestScore>beta){
		Statistics::instance().gatherNodeTypeStat(type,CUT_NODE);
	}else if(TTtype==typeExact){
		Statistics::instance().gatherNodeTypeStat(type,PV_NODE);
	}else{
		Statistics::instance().gatherNodeTypeStat(type,ALL_NODE);
	}
#endif

	//sync_cout<<"Qsearch Return:"<<bestScore<<sync_endl;
	return bestScore;

}


