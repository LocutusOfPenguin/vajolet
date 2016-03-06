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

#include <vector>
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
	Position&  pos;
	const mctsNode* const parent;
	const Move CurrentMove;

	std::vector<mctsNode> childrens;
	mctsNode(mctsNode* par,Position & p,const Move& m): pos(p),parent(par), CurrentMove(m){}
	void insertChildren(Move & m)
	{
		mctsNode newNode(this,pos,m);
		childrens.push_back(newNode);
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
	Move hasUntriedMoves(void)
	{
		Movegen mg(pos);
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
				return m;
			}

		}
		return Movegen::NOMOVE;
	}


};

class mcts
{


public:
	Position pos;
	mcts(): firstNode(nullptr,pos,Movegen::NOMOVE)
	{
		pos.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	}
	mctsNode firstNode;
	void print()
	{
		firstNode.printMoves();
	}


	mctsNode& selectNode()
	{
		Move m;
		while( (m = firstNode.hasUntriedMoves()) != Movegen::NOMOVE)
		{
			firstNode.insertChildren(m);
		}
		return firstNode;
	}





};

#endif
