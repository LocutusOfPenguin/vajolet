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
#include <iostream>
#include <fstream>
#include "position.h"
#include "search.h"
#include "transposition.h"
#include "command.h"
#include "movegen.h"
#include "eval.h"



struct timeManagementStruct
{
	volatile long long allocatedTime;
	volatile long long minSearchTime;
	volatile long long maxAllocatedTime;
	volatile unsigned int depth;
	volatile unsigned int singularRootMoveCount;
	volatile unsigned int resolution;
	volatile bool idLoopIterationFinished;
	volatile bool idLoopAlpha;
	volatile bool idLoopBeta;
//
	bool FirstIterationFinished;

};

class Tdleaf
{
public:
	struct tdleafData
	{
		std::string fen;
		double logistic;
		double dt;
		double multiplier;
		double sum;
		double derivata;
	};

	struct tdleafPars
	{
		simdScore* pointer;
		double value[4];
		tdleafPars(simdScore * v)
		{
			pointer = v;
			value[0] = (*v)[0];
			value[1] = (*v)[1];
			value[2] = (*v)[2];
			value[3] = (*v)[3];
		}
	};

	double lambda = 0.7;
	double alpha = 1.0;
	double Param;
	std::vector<tdleafData> data;
	std::vector<tdleafPars> params;
	Tdleaf(){
		params.push_back(tdleafPars(&isolatedPawnPenalty));
		params.push_back(tdleafPars(&isolatedPawnPenaltyOpp));
		params.push_back(tdleafPars(&doubledPawnPenalty));
		params.push_back(tdleafPars(&backwardPawnPenalty));
		params.push_back(tdleafPars(&chainedPawnBonus));
		params.push_back(tdleafPars(&passedPawnFileAHPenalty));
		params.push_back(tdleafPars(&passedPawnSupportedBonus));
		params.push_back(tdleafPars(&candidateBonus));
		params.push_back(tdleafPars(&passedPawnBonus));
		params.push_back(tdleafPars(&passedPawnUnsafeSquares));
		params.push_back(tdleafPars(&passedPawnBlockedSquares));
		params.push_back(tdleafPars(&passedPawnDefendedSquares));
		params.push_back(tdleafPars(&passedPawnDefendedBlockingSquare));
		params.push_back(tdleafPars(&unstoppablePassed));
		params.push_back(tdleafPars(&rookBehindPassedPawn));
		params.push_back(tdleafPars(&EnemyRookBehindPassedPawn));

		params.push_back(tdleafPars(&holesPenalty));
	}
};

class Game
{
	Tdleaf tdleaf;
public:
	struct GamePosition
	{
		U64 key;
		Move m;
		std::vector<Move> PV;
		Score alpha;
		Score beta;
		std::string fen;
		Score res;
		unsigned int depth;
	};
private:
	std::vector<GamePosition> positions;
public:
	unsigned int getGameLength()
	{
		return positions.size();
	}

	void CreateNewGame(void)
	{
		//sync_cout<<"info debug NEW GAME DETECTED"<<sync_endl;
		positions.clear();
	}

	void insertNewMoves(Position &pos)
	{
		unsigned int actualPosition = getGameLength();
		for(unsigned int i = actualPosition; i <= pos.getStateIndex(); i++)
		{
			//sync_cout<<"info debug new position inserted in game:"<<displayUci(pos.getState(i).currentMove)<<sync_endl;
			GamePosition p;
			p.key = pos.getState(i).key;
			p.m =  pos.getState(i).currentMove;
			p.fen = pos.getFen();
			positions.push_back(p);
		}
	}

	void savePV(std::list<Move> PV,unsigned int depth, Score alpha, Score beta, Score res)
	{
		std::copy(std::begin(PV), std::end(PV), std::back_inserter(positions.back().PV));
		positions.back().depth = depth;
		positions.back().alpha = alpha;
		positions.back().beta = beta;
		positions.back().res = res;

	}


	void printGamesInfo()
	{


		std::ofstream fout;
		fout.open("out.csv",std::ios::app);
		//fout<<"new iteration"<<std::endl;
		tdleaf.data.clear();
		float oldLogi = 0;
		float logi = 0;
		for(auto p : positions)
		{
			if( p.m != Movegen::NOMOVE)
			{
				std::cout<<"move leading here: "<<displayUci(p.m);
				if(p.PV.size()>0)
				{
					/*std::cout<<std::endl;
					Position ppp;
					ppp.setupFromFen(p.fen);
					ppp.display();*/
					/*std::cout<<" PV:";
					for( auto m : p.PV )
					{
						std::cout<<displayUci(m)<<" ";
					}*/
					logi = logistic(p.res);
					fout<<" res:"<<p.res/100<<" logistic:"<<logi <<" dt:"<<logi-oldLogi;

					//p.fen
					Position pp;
					pp.setupFromFen(p.fen);
					for( auto m : p.PV )
					{
						pp.doMove(m);
					}

					Tdleaf::tdleafData data;
					data.fen = pp.getFen();
					data.multiplier = p.PV.size()%2 ==0 ? 1.0 : -1.0;
					tdleaf.data.push_back(data);
				}

				fout<<std::endl;
				oldLogi = logi;
			}


		}

		Position p;
		for( auto& d :tdleaf.data)
		{
			p.setupFromFen(d.fen);
			d.logistic= d.multiplier * logistic(p.eval<false>());
		}
		for (unsigned int i = 0; i<tdleaf.data.size() - 1; i++)
		{
			float lt2 = tdleaf.data[i+1].logistic;
			auto  lt1 = tdleaf.data[i].logistic;
			tdleaf.data[i].dt = lt2 - lt1;
			fout<<tdleaf.data[i].fen<<" "<<tdleaf.data[i].dt<<sync_endl;
		}
		tdleaf.data.back().dt = 0;
		fout<<tdleaf.data.back().fen<<" "<<tdleaf.data.back().dt<<sync_endl;



		for (unsigned int i = 0; i<tdleaf.data.size(); i++)
		{
			double sum = 0;
			for (unsigned int n = i; n<tdleaf.data.size(); n++)
			{
				sum += std::pow(tdleaf.lambda,n-i) * tdleaf.data[n].dt;
			}
			tdleaf.data[i].sum = sum;
			fout<<tdleaf.data[i].fen<<" "<<tdleaf.data[i].dt<<" "<<sum<<sync_endl;
		}

		for (auto par: tdleaf.params)
		{

			for( int i=0 ;i<4;i++){
				for( auto& d :tdleaf.data)
				{
					p.setupFromFen(d.fen);
					int backup = (*par.pointer)[i];
					Score original = p.eval<false>();
					(*par.pointer)[i] = backup + 10;
					Score modified = p.eval<false>();
					(*par.pointer)[i] = backup ;
					double derivata = (modified - original)/10.0;
					d.derivata = derivata;

				}
				for( auto& d :tdleaf.data)
				{
					fout<<d.fen<<" "<<d.logistic<<" "<<d.dt<<" "<<d.sum<<" "<<d.derivata<<std::endl;
				}
				double delta = 0;
				for( auto& d :tdleaf.data)
				{
					delta += tdleaf.alpha * d.derivata * d.sum;
				}
				fout<<"delta: "<< delta;
				par.value[i] += delta;
				fout<<" "<< par.value[i] <<";"<<std::endl;
				(*par.pointer)[i] = par.value[i];
			}
		}

		fout<< std::endl;
	}

	float logistic(int res)
	{
		return float( 2.0 /(1.0 + exp(-0.00005 * res)) - 1.0);
	}


	~Game()
	{
		//printGamesInfo();
	}
	bool isNewGame(Position &pos)
	{
		if( getGameLength() == 0 || pos.getStateIndex()+1 <  getGameLength())
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
		if( getGameLength() > 2)
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
	Search src;

	static long long lastHasfullMessage;

	Game game;
	void initThreads();

	void timerThread();
	void searchThread();
	void manageNewSearch();
public :
	void quitThreads();
	void createNewGame();

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
	void startThinking(Position * p, searchLimits& l)
	{
		src.stop = true;
		lastHasfullMessage = 0;

		while(startThink){}

		if(!startThink)
		{
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
