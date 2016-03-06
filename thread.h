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

#ifndef THREAD_H_
#define THREAD_H_

#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include "position.h"
#include "search.h"
#include "transposition.h"
#include "command.h"
#include "movegen.h"



struct timeManagementStruct
{
	volatile unsigned long allocatedTime;
	//volatile unsigned long minSearchTime;
	//volatile unsigned long maxSearchTime;
	volatile unsigned int depth;
	volatile unsigned int singularRootMoveCount;
	volatile unsigned int resolution;
	volatile bool idLoopIterationFinished;

};


class Game
{
public:
	struct GamePosition
	{
		U64 key;
		Move m;
		std::vector<Move> PV;
		Score alpha;
		Score beta;
		unsigned int depth;
	};
private:
	std::vector<GamePosition> positions;
public:

	void CreateNewGame(void)
	{
		positions.clear();
	}

	void insertNewMoves(Position &pos)
	{
		unsigned int actualPosition = positions.size();
		for(unsigned int i = actualPosition; i <= pos.getStateIndex(); i++)
		{
			//sync_cout<<"inserita pos:"<<pos.getState(i).key<<sync_endl;
			GamePosition p;
			p.key = pos.getState(i).key;
			p.m =  pos.getState(i).currentMove;
			positions.push_back(p);
		}
	}

	void savePV(std::list<Move> PV,unsigned int depth, Score alpha, Score beta)
	{
		std::copy(std::begin(PV), std::end(PV), std::back_inserter(positions.back().PV));
		positions.back().depth = depth;
		positions.back().alpha = alpha;
		positions.back().beta = beta;
	}


	void printGamesInfo()
	{
		for(auto p : positions)
		{
			if( p.m != Movegen::NOMOVE)
			{
				std::cout<<"Move: "<<displayUci(p.m)<<"  PV:";
				for( auto m : p.PV )
				{
					std::cout<<displayUci(m)<<" ";
				}

			}
			std::cout<<std::endl;
		}

	}

	~Game()
	{
		//printGamesInfo();
	}
	bool isNewGame(Position &pos)
	{
		if( positions.size() == 0 || pos.getStateIndex()+1 < positions.size())
		{
			//printGamesInfo();
			//sync_cout<<"NEW GAME"<<sync_endl;
			return true;
		}

		unsigned int n = 0;
		for(auto p : positions)
		{
			if(pos.getState(n).key != p.key)
			{
				//printGamesInfo();
				//sync_cout<<"NEW GAME"<<sync_endl;
				return true;
			}
			n++;

		}
		return false;
	}

	bool isPonderRight(void)
	{
		if( positions.size() > 2)
		{
			GamePosition previous =*(positions.end()-3);
			if(previous.PV.size()>=1 && previous.PV[1] == positions.back().m)
			{
				//sync_cout<<"PONDER HIT"<<sync_endl;
				return true;
			}

		}
		//sync_cout<<"PONDER FAIL"<<sync_endl;
		return false;
	}

	GamePosition getNewSearchParameters(void)
	{
		GamePosition previous =*(positions.end()-3);
		return previous;
	}


};

class my_thread
{

	bool mcts = false;
	my_thread()
	{
		initThreads();
		game.CreateNewGame();
	};

	static my_thread * pInstance;


	volatile static bool quit;
	volatile static bool startThink;
	std::thread timer;
	std::thread searcher;
	std::mutex searchMutex;
	std::condition_variable searchCond;
	std::condition_variable timerCond;
	search src;

	static long long lastHasfullMessage;

	Game game;
	void initThreads();

	void timerThread();
	void searchThread();
	void manageNewSearch();
	void autoGame();
public :
	void quitThreads();

	static std::mutex  _mutex;

	static my_thread* getInstance()
	{
		if (!pInstance)
		{
			std::lock_guard<std::mutex> lock(_mutex);

			if (!pInstance)
			{
				my_thread * temp = new my_thread;
			    pInstance = temp;
			}
		}

		return pInstance;
	}

	static timeManagementStruct timeMan;

	~my_thread()
	{
		quitThreads();
	}
	void startThinking(Position * p, searchLimits& l,bool montecarlo = false)
	{

		src.stop = true;
		lastHasfullMessage = 0;

		while(startThink){}

		if(!startThink)
		{
			mcts = montecarlo;
			std::lock_guard<std::mutex> lk(searchMutex);
			src.limits = l;
			src.pos = *p;
			startThink = true;
			searchCond.notify_one();
		}
	}

	void stopThinking()
	{
		src.stop = true;
		src.stopPonder();
	}

	void ponderHit()
	{
		src.stopPonder();
	}

};




#endif /* THREAD_H_ */
