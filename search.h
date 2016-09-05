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
#ifndef SEARCH_H_
#define SEARCH_H_

#include <chrono>
#include <vector>
#include <list>
#include <atomic>
#include <cmath>
#include <string>
#include "vajolet.h"
#include "position.h"
#include "move.h"
#include "history.h"
#include "eval.h"


extern Move minhash[1 << 11];
struct startThinkResult
{
	Score alpha;
	Score beta;
	unsigned int depth;
	std::list<Move> PV;
};

class searchLimits
{
public:
	volatile bool ponder,infinite;
	unsigned int wtime,btime,winc,binc,movesToGo,depth,nodes,mate,moveTime;

	std::list<Move> searchMoves;
	searchLimits()
	{
		ponder = false;
		infinite = false;
		wtime = 0;
		btime = 0;
		winc = 0;
		binc = 0;
		movesToGo = 0;
		depth = 0;
		nodes = 0;
		mate = 0;
		moveTime = 0;
	}

};

class rootMove
{
public:
	Score score;
	Score previousScore;
	std::list<Move> PV;
	Move firstMove;
	unsigned int maxPlyReached;
	unsigned int depth;
	unsigned long long nodes;
	long long int time;
	bool operator<(const rootMove& m) const { return score > m.score; } // Ascending sort
	bool operator==(const Move& m) const { return firstMove.packed == m.packed; }
	void init(Move & m)
	{
		previousScore = -SCORE_INFINITE;
		score = -SCORE_INFINITE;
		firstMove = m;
		maxPlyReached = 0;
		depth = 0;
		nodes = 0;
		time = 0;
		PV.clear();
	}
};




class searchData
{
public:
	Move excludeMove;
	bool skipNullMove;
	Move killers[2];
};


class Search
{
private:
	static int globalReduction;
	bool useTBresult;
	static Score futility[5];
	static Score futilityMargin[7];
	static unsigned int FutilityMoveCounts[11];
	static Score PVreduction[32*ONE_PLY][64];
	static Score nonPVreduction[32*ONE_PLY][64];

	static Score mateIn(int ply) { return SCORE_MATE - ply; }
	static Score matedIn(int ply) { return SCORE_MATED + ply; }
	bool validIteration =false;

	unsigned int indexPV = 0;
	History history;
	CounterMove counterMoves;

	searchData sd[STATE_INFO_LENGTH];
	void cleanData(void)
	{
		std::memset(sd, 0, sizeof(sd));
	}

	void saveKillers(unsigned int ply, Move& m)
	{
		int h0, h1;
		pos.minHash(h0,h1);
		minhash[h0]= m;
		minhash[h1]= m;

		/*Move * const tempKillers =  sd[ply].killers;
		if(tempKillers[0] != m)
		{
			tempKillers[1] = tempKillers[0];
			tempKillers[0] = m;
		}*/

	}

	signed int razorMargin(unsigned int depth,bool cut) const { return 20000+depth*78+cut*20000; }


	enum nodeType
	{
		ROOT_NODE,
		HELPER_ROOT_NODE,
		PV_NODE,
		ALL_NODE,
		CUT_NODE
	} ;

	template<nodeType type>Score qsearch(unsigned int ply,int depth,Score alpha,Score beta,std::list<Move>& PV);
	template<nodeType type>Score alphaBeta(unsigned int ply,int depth,Score alpha,Score beta,std::list<Move>& PV);

	static std::atomic<unsigned long long> visitedNodes;

	unsigned int maxPlyReached;

	void reloadPv(unsigned int i);

public:
	static std::atomic<unsigned long long> tbHits;
	static std::vector<rootMove> rootMoves;
	searchLimits limits;
	Position pos;

	const History& getHistory()const {return history;}
	const CounterMove& getCounterMove()const {return  counterMoves;}



	static unsigned int threads;
	static unsigned int multiPVLines;
	static bool useOwnBook;
	static bool bestMoveBook;
	static bool showCurrentLine;
	static std::string SyzygyPath;
	static unsigned int SyzygyProbeDepth;
	static bool Syzygy50MoveRule;
	volatile bool showLine = false;

	static void initLMRreduction(void)
	{
		for (int d = 1; d < 32*ONE_PLY; d++)
			for (int mc = 1; mc < 64; mc++)
			{
				double    PVRed = -1.5 + 0.33*log(double(d)) * log(double(mc));
				double nonPVRed = -1.2 + 0.37*log(double(d)) * log(double(mc));
				PVreduction[d][mc] = (Score)(PVRed >= 1.0 ? floor(PVRed * int(ONE_PLY)) : 0);
				nonPVreduction[d][mc] = (Score)(nonPVRed >= 1.0 ? floor(nonPVRed * int(ONE_PLY)) : 0);
			}
	};

	void stopPonder(){ limits.ponder = false;}
	volatile bool stop = false;


	const Move&  getKillers(unsigned int ply,unsigned int n) const {
		/*if(n<2) return sd[ply].killers[n];*/
		int h0, h1;
		pos.minHash(h0,h1);
		if(n==0){
			return minhash[h0];

		}
		return minhash[h1];
	}


	startThinkResult startThinking(unsigned int depth = 1, Score alpha = -SCORE_INFINITE, Score beta = SCORE_INFINITE);
	unsigned long long getVisitedNodes() const { return visitedNodes; }

private:
	// gestione timer
	long long int startTime;
public:
	static long long int getTime(){ return std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count(); }
	long long int getElapsedTime() const { return getTime() - startTime; }
	void resetStartTime(){ startTime = getTime(); }

};

extern Search defaultSearch;

#endif /* SEARCH_H_ */
