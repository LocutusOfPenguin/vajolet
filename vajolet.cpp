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
#include "statistics.h"
#include "search.h"
#include "eval.h"
#include "tune.h"

/*!	\brief	print the startup information
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static void printStartInfo(void){
	sync_cout<<PROGRAM_NAME<<" "<<VERSION<<" by Marco Belli"<<sync_endl;
	/*bool hwpopcnt=__builtin_cpu_supports("popcnt");
	if(hwpopcnt){
		sync_cout<<"hw popcnt"<<sync_endl;
	}*/
}

/*!	\brief	main function
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
int main()
{
	// todo test for cpu type and capability con le nuove funzioni di gcc
	//
	//__builtin_cpu_supports("ssse3");
	//TODO __builtin_cpu_supports("popcnt")
	//TODO gcc Function Multiversioning per fare popcnt hw solo se c'�, si pu� usare con gcc 4.8.2
	//----------------------------------
	//	init global data
	//----------------------------------

	std::cout.rdbuf()->pubsetbuf(0,0);
	std::cin.rdbuf()->pubsetbuf(0,0);
	initData();
	HashKeys::init();
	Position::initScoreValues();
	Position::initCastleRightsMask();
	Movegen::initMovegenConstant();
	search::initLMRreduction();
	TT.setSize(1);
	initMaterialKeys();
	initMobilityBonus();

	//----------------------------------
	//	main loop
	//----------------------------------
	printStartInfo();

//	Tuner t;
//	t.simulatedAnnealing();
	//t.tuneParameters();
	//t.parseEpd(true);
	//t.createEpd();
	//t.drawSigmoid();
	//t.drawAverageEvolution();


#ifdef PRINT_STATISTICS
	Statistics::instance().initNodeTypeStat();
#endif

	uciLoop();





}
