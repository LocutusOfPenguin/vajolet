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

#ifndef MCTS_H_
#define MCTS_H_

#include <list>
#include <algorithm>
#include "move.h"
#include "movegen.h"
#include "position.h"
#include "io.h"
#include "command.h"
#include "transposition.h"

class mctsNode
{

public:
	float totalValue = 0;
	unsigned long totalVisit = 0;
	Position&  pos;
	const mctsNode* const parent;
	const Move CurrentMove;

	std::list<mctsNode> childrens;
	mctsNode(mctsNode* par,Position & p,const Move& m): pos(p),parent(par), CurrentMove(m){}
	mctsNode& insertChildren(Move & m)
	{
		mctsNode newNode(this,pos,m);
		childrens.push_back(newNode);
		return childrens.back();

	}
	void printMoves()
	{
		if(childrens.size()>0)
		{
			sync_cout<<"---NODE---"<<sync_endl;
			for (auto &i : childrens)
			{
				sync_cout<<"Move: "<<displayUci(i.CurrentMove)<<sync_endl;
				i.printMoves();
			}
		}
	}
	bool HasChildren()
	{
		return childrens.size()>0;
	}
	Move hasUntriedMoves(bool best = false)
	{
		ttEntry* tte = TT.probe(pos.getKey());
		Move ttMove;

		Score bestScore = -SCORE_INFINITE;

		Move choosenMove = Movegen::NOMOVE;
		ttMove = (tte != nullptr) ? tte->getPackedMove() : 0;
		Movegen mg(pos,defaultSearch, 0, 100*ONE_PLY, ttMove);
		Move m;
		while( (m = mg.getNextMove() ) != Movegen::NOMOVE)
		{
			bool found = false;
			auto iter = childrens.begin();
			while(iter != childrens.end() )
			{
				if((*iter).CurrentMove == m)
				{
					found = true;
				}
				iter++;
			}
			if( !found )
			{
				if(best)
				{
					pos.doMove(m);
					Score res = -pos.eval<false>();

					if(res >bestScore)
					{
						res = bestScore;
						choosenMove = m;
					}

					pos.undoMove();
				}
				else{
					return m;
				}

			}

		}
		return choosenMove;
	}


};

class mcts
{


public:
	Position& pos;
	mcts(Position& p ): pos(p),firstNode(nullptr,pos,Movegen::NOMOVE)
	{
	}
	mctsNode firstNode;
	void print()
	{
		struct temp
		{
			Move m;
			float winRate;
		};
		std::multimap<unsigned long, temp> map;
		for( auto child: firstNode.childrens)
		{
			temp t;
			t.m = child.CurrentMove;
			t.winRate = child.totalValue/child.totalVisit;
			map.insert(std::pair<unsigned long, temp>(child.totalVisit,t));

		}

		for( auto i: map)
		{
			sync_cout<<displayUci(i.second.m)<< " "<<i.first<<" "<<i.second.winRate<<sync_endl;
		}




	}

	void printBestMove(bool info=false)
	{
		unsigned long maxVisit =0;
		Move BestMove = Movegen::NOMOVE;
		for( auto child: firstNode.childrens)
		{
			if(child.totalVisit >maxVisit)
			{
				maxVisit = child.totalVisit;
				BestMove = child.CurrentMove;
			}

		}
		if(info)
		{
			sync_cout<<"info PV "<<displayUci(BestMove)<<sync_endl;
		}
		else
		{
			sync_cout<<"bestmove "<<displayUci(BestMove)<<sync_endl;

		}
	}
	void UCT()
	{
		for( int i =1;i<=200000;i++)
		{


			//sync_cout<<"Select "<<i<<sync_endl;
			mctsNode* node = selectNode();
			//std::cin.get();
			//sync_cout<<"Expand"<<sync_endl;
			node = expand(node);
			//std::cin.get();
			//sync_cout<<"Simulate"<<sync_endl;
			float result = simulate();
			//sync_cout<<result<<sync_endl;
			//std::cin.get();
			//sync_cout<<"Update"<<sync_endl;
			update(node,result);
			//std::cin.get();
			//sync_cout<<"----------------------------"<<sync_endl;
			if(i%100==0)
			{
				sync_cout<<"iteration: "<<i<<sync_endl;

				print();
				printBestMove(true);
			}
		}



	}


	void update(mctsNode* node,float result)
	{
		//sync_cout<<"--update--"<<sync_endl;
		do
		{


			node->totalValue += result;
			node->totalVisit ++;
			//sync_cout<<node->totalVisit<< " "<<node->totalValue<<sync_endl;
			if(node->parent)
			{
				pos.undoMove();
			}
			node = (mctsNode*)node->parent;

		}while(node);

	}
	mctsNode * selectNode()
	{

		Move m;
		mctsNode* currNode = &firstNode;
		mctsNode* choosen= currNode;

		//sync_cout<<"FIRST NODE"<<sync_endl;
		//sync_cout<<(currNode->totalVisit)<<"-->"<<(4+1*log(currNode->totalVisit+1))<<sync_endl;
		while ( currNode->HasChildren() && ((currNode->hasUntriedMoves()== Movegen::NOMOVE)|| (currNode != &firstNode && currNode->childrens.size()>(4+1*log(currNode->totalVisit+1))) ))
		{
			//sync_cout<<"new Node"<<sync_endl;
			float bestVal = -1e6;

			for(auto it = currNode->childrens.begin() ; it != currNode->childrens.end(); ++it)
			{

				//sync_cout<<"totalValue: "<< (*it).totalValue <<" totalVisit:"<<(*it).totalVisit<<sync_endl;
				float winningRate = ((*it).totalValue / (*it).totalVisit);
				Score score = pos.eval<false>();
				if(pos.getActualState().nextMove == Position::blackTurn)
				{
					winningRate = 1-winningRate;
					score = -score;
				}
				//sync_cout<<"winningRate: "<< winningRate <<sync_endl;
				float uctVal = winningRate+0.3*sqrt(2*log(currNode->totalVisit)/(*it).totalVisit)+score/(10000.0*(*it).totalVisit);
				/*if(currNode == &firstNode)
				{
				//sync_cout<<"totalVisit: "<< currNode->totalVisit <<" totalVisit:"<<(*it).totalVisit<<sync_endl;
					sync_cout<<"move:"<<displayUci(it->CurrentMove)<<sync_endl;
					sync_cout<<"uctVal: "<< uctVal <<sync_endl;
				}*/

				if(uctVal>=bestVal)
				{
					//sync_cout<<"best increased"<<sync_endl;
					bestVal = uctVal;
					choosen = &(*it);

				}
			}

			//sync_cout<<"----selected: "<<displayUci(choosen->CurrentMove)<<sync_endl;
			//std::cin.get();
			pos.doMove(choosen->CurrentMove);
			//pos.display();
			currNode = choosen;
			//sync_cout<<(currNode->totalVisit)<<"-->"<<(4+1*log(currNode->totalVisit+1))<<sync_endl;
		}

		//sync_cout<<"return"<<sync_endl;
		//currNode->printMoves();
		//sync_cout<<"-----------------"<<sync_endl;
		return currNode;
	}

	mctsNode * expand(mctsNode * node)
	{

		//sync_cout<<node<<sync_endl;
		//node->printMoves();

		ttEntry* tte = TT.probe(pos.getKey());
		Move ttMove;

		ttMove = (tte != nullptr) ? tte->getPackedMove() : 0;
		//sync_cout<<displayUci(ttMove)<<sync_endl;
		Movegen mg(pos,defaultSearch, 0, 100*ONE_PLY, ttMove);
		if( pos.isDraw(true) || mg.getNumberOfLegalMoves() == 0)
		{
			sync_cout<<"FINAL NODE: "<<sync_endl;
			return node;
		}

		Move m;
		if ((m = node->hasUntriedMoves())!= Movegen::NOMOVE)
		{
			//sync_cout<<"expanded: "<<displayUci(m)<<sync_endl;
			pos.doMove(m);

			mctsNode * n = &node->insertChildren(m);
			return n;
		}
		else
		{
			sync_cout<<"NO NODE: "<<displayUci(m)<<sync_endl;
			return node;
		}
	}

	float simulate()
	{

		defaultSearch.pos = pos;
		defaultSearch.limits.depth = 2;
		defaultSearch.verbose = false;
		//pos.display();
		//Position::eNextMove color = defaultSearch.pos.getActualState().nextMove;
		//sync_cout<<color<<sync_endl;

		//sync_cout<<"AUTOGAME"<<sync_endl;


		while(true)
		{
			//sync_cout<<"------------------------------------------"<<sync_endl;




			if( defaultSearch.pos.isDraw(true))
			{
				//defaultSearch.pos.display();
				//sync_cout<<"DRAW"<<sync_endl;
				return 0.5;
			}


			Movegen mg(defaultSearch.pos);
			if(mg.getNumberOfLegalMoves() == 0)
			{

				if(defaultSearch.pos.isInCheck())
				{
					//defaultSearch.pos.display();
					if(defaultSearch.pos.getActualState().nextMove == Position::blackTurn)
					{
						//sync_cout<<"white WIN"<<sync_endl;
						return 1;
					}
					else
					{
						//sync_cout<<"black WIN"<<sync_endl;
						return 0;
					}
				}
				else
				{
					//sync_cout<<"DRAW"<<sync_endl;
					return 0.5;
				}
			}

			startThinkResult res = defaultSearch.startThinking();
			//sync_cout <<"MOVE "<<displayUci(res.PV.front())<<sync_endl;
			defaultSearch.pos.doMove(res.PV.front());
		}

		//defaultSearch.pos.display();
		sync_cout<<"-1"<<sync_endl;
		return -1;
	}





};

#endif
