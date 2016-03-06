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
#include "command.h"
#include "io.h"
#include "bitops.h"
#include "data.h"
#include "hashKeys.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "search.h"
#include "eval.h"
#include "mcts.h"


/*!	\brief	print the startup information
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static void printStartInfo(void)
{
	sync_cout<<PROGRAM_NAME<<" "<<VERSION<<" by Marco Belli"<<sync_endl;
}

/*!	\brief	main function
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
int main()
{
	//----------------------------------
	//	init global data
	//----------------------------------
	std::cout.rdbuf()->pubsetbuf( 0, 0 );
	std::cin.rdbuf()->pubsetbuf( 0, 0 );
	initData();
	HashKeys::init();
	Position::initScoreValues();
	Position::initCastleRightsMask();
	Movegen::initMovegenConstant();
	search::initLMRreduction();
	TT.setSize(1);
	Position::initMaterialKeys();
	initMobilityBonus();

	mcts mc;
	mc.selectNode();
	mc.print();

	/*Move m;
	m.packed = 0;
	m.bit.from = E2;
	m.bit.to = E4;
	mc.firstNode.insertChildren(m);
	m.bit.from = D2;
	m.bit.to = D4;
	mc.firstNode.insertChildren(m);

	mctsNode& node = mc.firstNode.childrens[1];
	m.bit.from = E7;
	m.bit.to = E5;
	node.insertChildren(m);
	m.bit.from = D7;
	m.bit.to = D5;
	node.insertChildren(m);

	mctsNode& node2 = mc.firstNode.childrens[0];
	m.bit.from = A7;
	m.bit.to = A5;
	node2.insertChildren(m);
	m.bit.from = B7;
	m.bit.to = B5;
	node2.insertChildren(m);

	mc.print();*/




	//----------------------------------
	//	main loop
	//----------------------------------
	printStartInfo();
	uciLoop();
	return 0;
}
