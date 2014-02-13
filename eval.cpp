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

#include <utility>
#include <unordered_map>
#include <map>
#include <iomanip>
#include <algorithm>
#include "position.h"
#include "move.h"
#include "bitops.h"
#include "movegen.h"
#include "eval.h"


enum color{
	white=0,
	black=1
};

simdScore mobilityBonus[Position::separationBitmap][32];/* = {
		{}, {},
		{ simdScore(-1352,-2686,0,0), simdScore( -901,-1747,0,0), simdScore(-563, -939,0,0), simdScore(-225, -134,0,0), simdScore( 112,  670,0,0), simdScore( 450, 1475,0,0), // Queens
				simdScore(  788, 2284,0,0), simdScore(1127, 3089,0,0), simdScore(1468, 3894,0,0), simdScore(1806, 4568,0,0), simdScore(2031, 5104,0,0), simdScore(2257, 5373,0,0),
				simdScore( 2482, 5507,0,0), simdScore( 2595,5507,0,0), simdScore(2707, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0),
				simdScore( 2820, 5507,0,0), simdScore( 2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0),
				simdScore( 2820, 5507,0,0), simdScore( 2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0),
				simdScore( 2820, 5507,0,0), simdScore( 2820, 5507,0,0) },
		{ simdScore(-1918,-2217,0,0), simdScore(-1239,-1075,0,0), simdScore(-563,  0,0,0), simdScore( 112, 1075,0,0), simdScore( 788, 2150,0,0), simdScore(1465, 3225,0,0), // Rooks
				simdScore( 2031, 4300,0,0), simdScore(2482, 5375,0,0), simdScore(2933, 6450,0,0), simdScore(3271,7322,0,0), simdScore(3496,7726,0,0), simdScore(3725,7994,0,0),
				simdScore( 3835,8195,0,0), simdScore( 4063,8329,0,0), simdScore(4176,8329,0,0), simdScore(4288,8329,0,0) },
		{ simdScore(-2483,-3628,0,0), simdScore( -903,-1746,0,0), simdScore( 677,  134,0,0), simdScore(2257, 2015,0,0), simdScore(3838, 3896,0,0), simdScore(5418, 5778,0,0), // Bishops
				simdScore( 6773, 7390,0,0), simdScore( 7676, 8465,0,0), simdScore(8353, 9137,0,0), simdScore(8692,9675,0,0), simdScore(9031, 10078,0,0), simdScore(9257, 10346,0,0),
				simdScore( 9482, 10615,0,0), simdScore( 9708, 10884,0,0), simdScore(9821, 11018,0,0), simdScore(9821, 11018,0,0) },
		{ simdScore(-3951,-300,0,0), simdScore(-2843,-200,0,0), simdScore(-1016,-100,0,0), simdScore( 338,  0,0,0), simdScore(1693, 100,0,0), simdScore(3048, 200,0,0), // Knights
				simdScore( 4176, 280,0,0), simdScore( 4741, 310,0,0), simdScore(4967, 330,0,0) }
};*/

const int KingExposed[] = {
     2,  0,  2,  5,  5,  2,  0,  2,
     2,  2,  4,  8,  8,  4,  2,  2,
     7, 10, 12, 12, 12, 12, 10,  7,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15
  };



//------------------------------------------------
//	PAWN Bonus/Penalties
//------------------------------------------------
simdScore isolatedPawnPenalty={2000,2000,0,0};
simdScore isolatedPawnPenaltyOpp={1000,2000,0,0};
simdScore doubledPawnPenalty={500,500,0,0};
simdScore backwardPawnPenalty={600,600,0,0};
simdScore chainedPawnBonus={1200,1100,0,0};
simdScore passedPawnFileAHPenalty = {0,2000,0,0};
simdScore passedPawnSupportedBonus = {0,1000,0,0};
simdScore candidateBonus = {1000,500,0,0};


simdScore rookOn7Bonus={5700,3600,0,0};
simdScore rookOnPawns={1000,3000,0,0};
simdScore queenOn7Bonus={200,1600,0,0};
simdScore queenOnPawns={500,1000,0,0};
simdScore rookOnOpen={2000,500,0,0};
simdScore rookOnSemi={1000,500,0,0};

simdScore knightOnOutpost= {100,120,0,0};
simdScore knightOnOutpostSupported= {210,190,0,0};
simdScore knightOnHole= {310,390,0,0};

simdScore bishopOnOutpost= {80,110,0,0};
simdScore bishopOnOutpostSupported= {1900,170,0,0};
simdScore bishopOnHole= {290,370,0,0};

simdScore tempo= {700,700,0,0};

simdScore attackedByPawnPenalty[Position::separationBitmap]=
{	{0,0,0,0},
	{0,0,0,0},//king
	{2500,3200,0,0},//queen
	{1325,1527,0,0},//rook
	{1300,1450,0,0},//bishop
	{1200,1350,0,0},//knight
	{0,0,0,0},//pawn
	{0,0,0,0},
};

simdScore weakPiecePenalty[Position::separationBitmap][Position::separationBitmap]=
{	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0},	{0,0,0,0}},
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0},	{0,0,0,0}},//king
	{{0,0,0,0},{0,0,0,0},	{600,400,0,0},		{802,420,0,0},	{1200,1000,0,0},{1200,1000,0,0},{3200,2000,0,0},	{0,0,0,0}},//queen
	{{0,0,0,0},{0,0,0,0},	{400,200,0,0},		{420,320,0,0},	{800,600,0,0},	{700,400,0,0},	{2400,2000,0,0},	{0,0,0,0}},//rook
	{{0,0,0,0},{0,0,0,0},	{200,100,0,0},		{220,150,0,0},	{400,200,0,0},	{500,300,0,0},	{1000,890,0,0},	{0,0,0,0}},//bishop
	{{0,0,0,0},{0,0,0,0},	{200,100,0,0},	{221,147,0,0},	{423,217,0,0},	{423,417,0,0},	{890,954,0,0},	{0,0,0,0}},//knight
	{{0,0,0,0},{0,0,0,0},	{50,40,0,0},	{60,50,0,0},	{100,80,0,0},	{150,90,0,0},	{200,170,0,0},	{0,0,0,0}},//pawn
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},	{0,0,0,0},	{0,0,0,0},	{0,0,0,0}}
//						king				queen						rook					bishop				knight				pawn
};
//------------------------------------------------
//king safety
//------------------------------------------------
const unsigned int KingAttackWeights[] = { 0, 0, 5, 3, 2, 2 };
int kingShieldBonus= 1600;
int kingFarShieldBonus= 1000;
int kingStormBonus= 100;
//------------------------------------------------




//---------------------------------------------
//	MATERIAL KEYS
//---------------------------------------------


typedef struct{
	enum {
		exact,
		multiplicativeFunction,
		exactFunction,
		saturationH,
		saturationL
	}type ;
	bool (*pointer)(const Position & ,Score &);
	Score val;

}materialStruct;

std::unordered_map<U64, materialStruct> materialKeyMap;

bool evalKBPvsK(const Position& p, Score& res){
	color Pcolor=p.bitBoard[Position::whitePawns]?white:black;
	tSquare pawnSquare;
	tSquare bishopSquare;
	if(Pcolor==white){
		pawnSquare = p.pieceList[Position::whitePawns][0];
		int pawnFile=FILES[pawnSquare];
		if(pawnFile==0 || pawnFile==7){
			bishopSquare=p.pieceList[Position::whiteBishops][0];
			if( SQUARE_COLOR[BOARDINDEX[pawnFile][7]]!=SQUARE_COLOR[bishopSquare]){
				tSquare kingSquare=  p.pieceList[Position::blackKing][0];
				if(RANKS[kingSquare]>=6  && abs(pawnFile-FILES[kingSquare])<=1){
					res=0;
					return true;

				}
			}
		}
	}
	else{
		pawnSquare = p.pieceList[Position::blackPawns][0];
		int pawnFile=FILES[pawnSquare];
		if(pawnFile==0 || pawnFile==7){
			bishopSquare=p.pieceList[Position::blackBishops][0];
			if( SQUARE_COLOR[BOARDINDEX[pawnFile][0]]!=SQUARE_COLOR[bishopSquare]){
				tSquare kingSquare=  p.pieceList[Position::whiteKing][0];
				if(RANKS[kingSquare]<=1  && abs(pawnFile-FILES[kingSquare])<=1){
					res=0;
					return true;

				}
			}
		}
	}
	return false;

}

bool evalKBNvsK(const Position& p, Score& res){
	color color=p.bitBoard[Position::whiteBishops]?white:black;
	tSquare bishopSquare;
	tSquare kingSquare;
	tSquare enemySquare;
	tSquare mateSquare1,mateSquare2;
	int mul=1;
	if(color==white){
		mul=1;
		bishopSquare = p.pieceList[Position::whiteBishops][0];
		kingSquare = p.pieceList[Position::whiteKing][0];
		enemySquare = p.pieceList[Position::blackKing][0];
	}
	else{
		mul=-1;
		bishopSquare = p.pieceList[Position::blackBishops][0];
		kingSquare = p.pieceList[Position::blackKing][0];
		enemySquare = p.pieceList[Position::whiteKing][0];

	}
	int mateColor =SQUARE_COLOR[bishopSquare];
	if(mateColor== 0){
		mateSquare1=A1;
		mateSquare2=H8;
	}else{
		mateSquare1=A8;
		mateSquare2=H1;
	}

	res= SCORE_KNOWN_WIN+50000;
	//sync_cout<<"dis1="<<SQUARE_DISTANCE[enemySquare][kingSquare]<<sync_endl;
	//sync_cout<<"dis2="<<SQUARE_DISTANCE[enemySquare][mateSquare1]<<sync_endl;
	//sync_cout<<"dis3="<<SQUARE_DISTANCE[enemySquare][mateSquare2]<<sync_endl;
	res -= 5*SQUARE_DISTANCE[enemySquare][kingSquare];// devo tenere il re vicino
	res -= 10*std::min(SQUARE_DISTANCE[enemySquare][mateSquare1],SQUARE_DISTANCE[enemySquare][mateSquare2]);// devo portare il re avversario nel giusto angolo



	res *=mul;
	return true;

}

bool evalOppositeBishopEndgame(const Position& p, Score& res){
	if(SQUARE_COLOR[p.pieceList[Position::blackBishops][0]] !=SQUARE_COLOR[ p.pieceList[Position::whiteBishops][0]]){
		res=128;
		return true;
	}
	return false;

}

bool evalKRvsKm(const Position& p, Score& res){
		res=64;
		return true;
}



void initMobilityBonus(void){
	for(int i=0;i<Position::separationBitmap;i++){
		for(int n=0;n<32;n++){
			mobilityBonus[i][n] =simdScore{0,0,0,0};
		}
	}

	double n0=3;
	double m0=0;

	double n1=3;
	double m1=0;

	for(int n=0;n<32;n++){
		n0=5;
		n1=5;
		m0=2400.0/32.0;
		m1=4800.0/32.0;
		mobilityBonus[Position::Queens][n] =simdScore{m0*(n-n0),m1*(n-n1),0,0};
	}
	for(int n=0;n<32;n++){
		n0=5;
		n1=5;
		m0=2400.0/16.0;
		m1=4800.0/16.0;
		mobilityBonus[Position::Rooks][n] =simdScore{m0*(n-n0),m1*(n-n1),0,0};
	}
	for(int n=0;n<32;n++){
		n0=4;
		n1=4;
		m0=5500.0/16.0;
		m1=5500.0/16.0;
		mobilityBonus[Position::Bishops][n] =simdScore{m0*(n-n0),m1*(n-n1),0,0};
	}
	for(int n=0;n<32;n++){
		n0=3;
		n1=3;
		m0=2800/8.0;
		m1=2800/8.0;
		mobilityBonus[Position::Knights][n] =simdScore{m0*(n-n0),m1*(n-n1),0,0};
	}


}

void initMaterialKeys(void){
	/*---------------------------------------------
	 *
	 * K vs K		->	draw
	 * km vs k		->	draw
	 * km vs km		->	draw
	 * kmm vs km	->	draw
	 *
	 * kbp vs k		->	probable draw on the rook file
	 *
	 * kb vs kpawns	-> bishop cannot win
	 * kn vs kpawns	-> bishop cannot win
	 *
	 * kbn vs k		-> win
	 * opposite bishop endgame -> look drawish
	 * kr vs km		-> look drawish
	 ----------------------------------------------*/


	Position p;
	U64 key;
	//------------------------------------------
	//	k vs K
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	materialStruct t;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs K
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs K
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	k vs KB
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	k vs KN
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});


	//------------------------------------------
	//	kn vs KB
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KN
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs KB
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs KN
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbb vs KN
	//------------------------------------------
	p.setupFromFen("kbb5/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbb vs KB
	//------------------------------------------
	p.setupFromFen("kbb5/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbn vs KN
	//------------------------------------------
	p.setupFromFen("kbn5/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbn vs KB
	//------------------------------------------
	p.setupFromFen("kbn5/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	knn vs KN
	//------------------------------------------
	p.setupFromFen("knn5/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	knn vs KB
	//------------------------------------------
	p.setupFromFen("knn5/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs KBB
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/5BBK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KBB
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/5BBK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs KBN
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/5BNK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KBN
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/5BNK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs KNN
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/5NNK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KNN
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/5NNK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs Kpawns
	//------------------------------------------
	t.type=materialStruct::saturationL;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("kb6/8/8/8/8/8/7P/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/6PP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/5PPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/4PPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/3PPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/2PPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/1PPPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/PPPPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});

	t.type=materialStruct::saturationH;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("k7/8/8/8/8/8/7p/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/6pp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/5ppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/4pppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/3ppppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/2pppppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/1ppppppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/ppppppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});


	//------------------------------------------
	//	kn vs Kpawns
	//------------------------------------------
	t.type=materialStruct::saturationL;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("kn6/8/8/8/8/8/7P/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/6PP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/5PPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/4PPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/3PPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/2PPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/1PPPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/PPPPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});

	t.type=materialStruct::saturationH;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("k7/8/8/8/8/8/7p/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/6pp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/5ppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/4pppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/3ppppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/2pppppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/1ppppppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/ppppppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});







	//------------------------------------------
	//	k vs KBP
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/5BPK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBPvsK;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbp vs K
	//------------------------------------------
	p.setupFromFen("kbp5/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBPvsK;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	k vs KBN
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/5BNK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBNvsK;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbn vs K
	//------------------------------------------
	p.setupFromFen("kbn5/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBNvsK;
	t.val=0;
	materialKeyMap.insert({key,t});


	//------------------------------------------
	//	opposite bishop endgame
	//------------------------------------------
	for(int wp=0;wp<=8;wp++){
		for(int bp=0;bp<=8;bp++){
			if(wp!=0 || bp !=0){
				std::string s="kb6/";
				for(int i=0;i<bp;i++){
					s+="p";
				}
				if(bp!=8){s+=std::to_string(8-bp);}
				s+="/8/8/8/8/";
				for(int i=0;i<wp;i++){
					s+="P";
				}
				if(wp!=8){s+=std::to_string(8-wp);}
				s+="/6BK w - -";
				//sync_cout<<s<<sync_endl;
				p.setupFromFen(s);
				key=p.getActualState().materialKey;
				t.type=materialStruct::multiplicativeFunction;
				t.pointer=&evalOppositeBishopEndgame;
				t.val=0;
				materialKeyMap.insert({key,t});
			}
		}
	}

	//------------------------------------------
	//	kr vs KN
	//------------------------------------------
	p.setupFromFen("kr6/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kr vs KB
	//------------------------------------------
	p.setupFromFen("kr6/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs KR
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/6RK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KR
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/6RK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});
}


//---------------------------------------------
const materialStruct* getMaterialData(const Position& p){
	U64 key=p.getActualState().materialKey;

	std::unordered_map<U64,materialStruct>::const_iterator got= materialKeyMap.find(key);
	if(got == materialKeyMap.end())
	{
		return nullptr;
	}
	else
	{
		 return &(got->second);
	}

}




template<color c>
simdScore evalPawn(const Position & p,tSquare sq, bitMap & weakPawns, bitMap & passedPawns){

	simdScore res=simdScore{0,0,0,0};

	bool passed, isolated, doubled, opposed, chain, backward;
	const bitMap ourPawns =c?p.bitBoard[Position::blackPawns]:p.bitBoard[Position::whitePawns];
	const bitMap theirPawns =c?p.bitBoard[Position::whitePawns]:p.bitBoard[Position::blackPawns];
	bitMap b;
	const int relativeRank =c?7-RANKS[sq] :RANKS[sq];
	//const int file =FILES[sq];

	// Our rank plus previous one. Used for chain detection
    b = RANKMASK[sq] | RANKMASK[sq-pawnPush(c)];
    // Flag the pawn as passed, isolated, doubled or member of a pawn
    // chain (but not the backward one).
    chain    = (ourPawns   & ISOLATED_PAWN[sq] & b);
	isolated = !(ourPawns   & ISOLATED_PAWN[sq]);
    doubled  = (ourPawns   & SQUARES_IN_FRONT_OF[c][sq])!=0;
    opposed  = (theirPawns & SQUARES_IN_FRONT_OF[c][sq])!=0;
    passed   = (theirPawns & PASSED_PAWN[c][sq])==0;

	backward=false;
	if(
		!(passed | isolated | chain) &&
		!(ourPawns & PASSED_PAWN[1-c][sq+pawnPush(c)] & ISOLATED_PAWN[sq]))// non ci sono nostri pedoni dietro a noi
	{
		b = RANKMASK[sq+pawnPush(c)]& ISOLATED_PAWN[sq];
		while (!(b & (ourPawns | theirPawns))){
			if(!c){
				b <<= 8;
			}
			else{
				b >>= 8;
			}

		}
		backward = ((b | ((!c)?(b << 8):(b >> 8))) & theirPawns)!=0;



	}

	if (isolated){
		if(opposed){
			res -= isolatedPawnPenaltyOpp;
		}
		else{
			res -= isolatedPawnPenalty;
		}
		weakPawns|=BITSET[sq];

	}

    if (doubled){
    	res-= doubledPawnPenalty;
	//	weakPawns|=BITSET[sq];
	}

    if (backward){
    	if(opposed){
			res -= backwardPawnPenalty/2;
		}
		else{
			res -= backwardPawnPenalty;
		}
		weakPawns|=BITSET[sq];
	}

    if (chain){
    	res+= chainedPawnBonus;
	}


	//passed pawn
	if(passed &&!doubled){
		/*int r=relativeRank-1;
		int rr= r*(r-1);

		res+=simdScore(600*rr,1100*(rr+r+1),0,0);*/

		passedPawns|=BITSET[sq];

		/*if(file==0 || file==7){
			res -=passedPawnFileAHPenalty;
		}
		if(chain){
			res+=passedPawnSupportedBonus*r;
		}*/
	}

	if ( !passed && !isolated && !doubled && !opposed && bitCnt( PASSED_PAWN[c][sq] & theirPawns ) < bitCnt(PASSED_PAWN[c][sq-pawnPush(c)] & ourPawns ))
	{
		res+=candidateBonus*(relativeRank-1);
	}
	return res;
}

template<Position::bitboardIndex piece>
simdScore evalPieces(const Position & p, const bitMap * const weakSquares,  bitMap * const attackedSquares ,const bitMap * const holes, bitMap * const kingRing,unsigned int * const kingAttackersCount,unsigned int * const kingAttackersWeight,unsigned int * const kingAdjacentZoneAttacksCount){
	simdScore res=simdScore{0,0,0,0};
	bitMap tempPieces=p.bitBoard[piece];
	bitMap enemyKing =(piece>Position::separationBitmap)? p.bitBoard[Position::whiteKing]:p.bitBoard[Position::blackKing];
	tSquare enemyKingSquare =(piece>Position::separationBitmap)? p.pieceList[Position::whiteKing][0]:p.pieceList[Position::blackKing][0];
	bitMap enemyKingRing =(piece>Position::separationBitmap)? kingRing[white]:kingRing[black];
	unsigned int * pKingAttackersCount=(piece>Position::separationBitmap)?&kingAttackersCount[black]:&kingAttackersCount[white];
	unsigned int * pkingAttackersWeight=(piece>Position::separationBitmap)?&kingAttackersWeight[black]:&kingAttackersWeight[white];
	unsigned int * pkingAdjacentZoneAttacksCount=(piece>Position::separationBitmap)?&kingAdjacentZoneAttacksCount[black]:&kingAdjacentZoneAttacksCount[white];
	const bitMap & enemyWeakSquares=(piece>Position::separationBitmap)? weakSquares[white]:weakSquares[black];
	const bitMap & enemyHoles=(piece>Position::separationBitmap)? holes[white]:holes[black];
	const bitMap & supportedSquares=(piece>Position::separationBitmap)? attackedSquares[Position::blackPawns]:attackedSquares[Position::whitePawns];
	const bitMap & threatenSquares=(piece>Position::separationBitmap)? attackedSquares[Position::whitePawns]:attackedSquares[Position::blackPawns];
	const bitMap ourPieces=(piece>Position::separationBitmap)? p.bitBoard[Position::blackPieces]:p.bitBoard[Position::whitePieces];
	while(tempPieces){
		tSquare sq=firstOne(tempPieces);
		tempPieces&= tempPieces-1;
		unsigned int relativeRank =(piece>Position::separationBitmap)? 7-RANKS[sq]:RANKS[sq];

		//---------------------------
		//	MOBILITY
		//---------------------------
		// todo mobility usare solo mosse valide ( pinned pieces)
		//todo mobility with pinned, mobility contando meno case attaccate da pezzi meno forti
		// todo fare una tabella per ogni pezzo



		bitMap attack;


		switch(piece){

		case Position::whiteRooks:
			attack = Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]^p.bitBoard[Position::whiteRooks]^p.bitBoard[Position::whiteQueens]);
			break;
		case Position::blackRooks:
			attack = Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]^p.bitBoard[Position::blackRooks]^p.bitBoard[Position::blackQueens]);
			break;
		case Position::whiteBishops:
			attack = Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]^p.bitBoard[Position::whiteQueens]);
			break;
		case Position::blackBishops:
			attack = Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]^p.bitBoard[Position::blackQueens]);
			break;
		case Position::whiteQueens:
		case Position::blackQueens:
		case Position::whiteKnights:
		case Position::blackKnights:
			attack = Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]);
		default:
			break;
		}

		if(attack & enemyKingRing){
			(*pKingAttackersCount)++;
			(*pkingAttackersWeight)+=KingAttackWeights[piece%Position::separationBitmap];
			bitMap adjacent=attack& Movegen::attackFromKing(enemyKingSquare);
			if(adjacent)
			{
				(*pkingAdjacentZoneAttacksCount)+=bitCnt(adjacent);
			}
		}
		attackedSquares[piece]|=attack;


		bitMap defendedPieces=attack&ourPieces;
		// piece coordination
		res+=(int)bitCnt(defendedPieces)*simdScore{200,180,0,0};

		unsigned int mobility= bitCnt(attack&~(threatenSquares/*|ourPieces*/));
		//sync_cout<<mobility<<sync_endl;
		res+=mobilityBonus[piece%Position::separationBitmap][mobility];
		if(!(attack&~(threatenSquares|ourPieces)) && (threatenSquares&bitSet(sq))){ // zero mobility && attacked by pawn
			res-=(Position::pieceValue[piece%Position::separationBitmap]/4);
		}
		/////////////////////////////////////////
		// center control
		/////////////////////////////////////////
		if(attack & centerBitmap){
			res+=(int)bitCnt(attack & centerBitmap)*simdScore{400,0,0,0};
		}
		if(attack & bigCenterBitmap){
			res+=(int)bitCnt(attack & bigCenterBitmap)*simdScore{100,0,0,0};
		}

		//todo alfiere buono cattivo
		switch(piece){
		case Position::whiteQueens:
		case Position::blackQueens:
		{
			bitMap enemyBackRank=(piece>Position::separationBitmap)? RANKMASK[A1]:RANKMASK[A8];
			bitMap enemyPawns=(piece>Position::separationBitmap)? p.bitBoard[Position::whitePawns]:p.bitBoard[Position::blackPawns];
			//--------------------------------
			// donna in 7a con re in 8a
			//--------------------------------
			if(relativeRank==6 &&(enemyKing & enemyBackRank) ){
				res+=queenOn7Bonus;
			}
			//--------------------------------
			// donna su traversa che contiene pedoni
			//--------------------------------
			if(relativeRank>4 && (RANKMASK[sq]& enemyPawns)){
				res+=queenOnPawns;
			}
			break;
		}
		case Position::whiteRooks:
		case Position::blackRooks:
		{
			bitMap enemyBackRank=(piece>Position::separationBitmap)? RANKMASK[A1]:RANKMASK[A8];
			bitMap enemyPawns=(piece>Position::separationBitmap)? p.bitBoard[Position::whitePawns]:p.bitBoard[Position::blackPawns];
			bitMap ourPawns=(piece>Position::separationBitmap)? p.bitBoard[Position::blackPawns]:p.bitBoard[Position::whitePawns];
			//--------------------------------
			// torre in 7a con re in 8a
			//--------------------------------
			if(relativeRank==6 &&(enemyKing & enemyBackRank) ){
				res+=rookOn7Bonus;
			}
			//--------------------------------
			// torre su traversa che contiene pedoni
			//--------------------------------
			if(relativeRank>4 && (RANKMASK[sq]& enemyPawns)){
				res+=rookOnPawns;
			}
			//--------------------------------
			// torre su colonna aperta/semiaperta
			//--------------------------------
			if(!(FILEMASK[sq]& ourPawns))
			{
				if(!(FILEMASK[sq]& enemyPawns))
				{
					res+=rookOnOpen;
				}else
				{
					res+=rookOnSemi;
				}
			}
			break;
		}
		case Position::whiteBishops:
		case Position::blackBishops:
			if(relativeRank>=4 && (enemyWeakSquares& BITSET[sq]))
			{
				res+=bishopOnOutpost;
				if(supportedSquares &BITSET[sq]){
					res += bishopOnOutpostSupported;
				}
				if(enemyHoles &BITSET[sq]){
					res += bishopOnHole;
				}

			}

			break;
		case Position::whiteKnights:
		case Position::blackKnights:
			if(relativeRank>=4 && (enemyWeakSquares& BITSET[sq]))
			{
				res+=knightOnOutpost;
				if(supportedSquares &BITSET[sq]){
					res += knightOnOutpostSupported;
				}
				if(enemyHoles &BITSET[sq]){
					res += knightOnHole;
				}

			}
			break;
		default:
			break;
		}
	}
	return res;
}


/*! \brief do a pretty simple evalutation
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
template<bool trace>
Score Position::eval(pawnTable& pawnHashTable, evalTable& evalTable) const {

	state &st =getActualState();
	evalEntry* probeEval= evalTable.probe(st.key);
	if(probeEval!=nullptr){
		if(st.nextMove)
		{
			return -probeEval->res;
		}
		else{
			return probeEval->res;
		}
	}

	simdScore traceRes=simdScore{0,0,0,0};
	if(trace){

		sync_cout <<std::setprecision(3)<< std::setw(21) << "Eval term " << "|     White    |     Black    |      Total     \n"
			      <<             "                     |    MG    EG  |   MG     EG  |   MG      EG   \n"
			      <<             "---------------------+--------------+--------------+-----------------"<<sync_endl;
	}

	Score lowSat=-SCORE_INFINITE;
	Score highSat=SCORE_INFINITE;
	Score mulCoeff=256;

	bitMap attackedSquares[lastBitboard]={0};
	bitMap weakSquares[2]={0};
	bitMap holes[2]={0};
	bitMap kingRing[2]={0};
	bitMap kingShield[2]={0};
	bitMap kingFarShield[2]={0};
	unsigned int kingAttackersCount[2]={0};
	unsigned int kingAttackersWeight[2]={0};
	unsigned int kingAdjacentZoneAttacksCount[2]={0};

	tSquare k=pieceList[whiteKing][0];
	kingRing[white]=Movegen::attackFromKing(k);
	kingShield[white]=kingRing[white];
	if(RANKS[k]<7){kingRing[white]|=Movegen::attackFromKing(tSquare(k+8));}
	kingFarShield[white]=kingRing[white]&~(kingShield[white]|BITSET[k]);


	k=pieceList[blackKing][0];
	kingRing[black]=Movegen::attackFromKing(k);
	kingShield[black]=kingRing[black];
	if(RANKS[k]>0){kingRing[black]|=Movegen::attackFromKing(tSquare(k-8));}
	kingFarShield[black]=kingRing[black]&~(kingShield[black]|BITSET[k]);




	// todo modificare valori material value & pst
	// material + pst

	simdScore res=simdScore{st.material[0],st.material[1],0,0};



	//-----------------------------------------------------
	//	material evalutation
	//-----------------------------------------------------
	const materialStruct* materialData=getMaterialData(*this);
	if(materialData)
	{
		switch(materialData->type){
		case materialStruct::exact:
			return materialData->val;
			break;
		case materialStruct::multiplicativeFunction:{
			Score r;
			if(materialData->pointer(*this,r)){
				mulCoeff=r;
			}

			break;
		}
		case materialStruct::exactFunction:
		{	Score r;

			if(materialData->pointer(*this,r)){
				return r;
			}
			break;
		}
		case materialStruct::saturationH:
			highSat=materialData->val;
			break;
		case materialStruct::saturationL:
			lowSat=materialData->val;
					break;
		}
	}

	//---------------------------------------------
	//	tempo
	//---------------------------------------------
	if(st.nextMove)
	{
		res-=tempo;
	}
	else{
		res+=tempo;
	}

	if(trace){
		sync_cout << std::setw(20) << "Material, PST, Tempo" << " |   ---    --- |   ---    --- | "
		          << std::setw(6)  << res[0]/10000.0 << " "
		          << std::setw(6)  << res[1]/10000.0 << " "<<sync_endl;
		traceRes=res;
	}


	//---------------------------------------------
	//	imbalancies
	//---------------------------------------------
	//	bishop pair

	if(pieceCount[whiteBishops]>=2 ){
		if(pieceCount[whiteBishops]==2 && SQUARE_COLOR[pieceList[whiteBishops][0]]==SQUARE_COLOR[pieceList[whiteBishops][1]]){
		}
		else{
			res+=simdScore{5000,5500,0,0};
		}
	}

	if(pieceCount[blackBishops]>=2 ){
		if(pieceCount[blackBishops]==2 && SQUARE_COLOR[pieceList[blackBishops][0]]==SQUARE_COLOR[pieceList[blackBishops][1]]){
		}
		else{
			res-=simdScore{5000,5500,0,0};
		}
	}
	if(trace){
		sync_cout << std::setw(20) << "imbalancies" << " |   ---    --- |   ---    --- | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}





	//todo specialized endgame & scaling function
	//todo material imbalance
	bitMap weakPawns=0;
	bitMap passedPawns=0;



	//----------------------------------------------
	//	PAWNS EVALUTATION
	//----------------------------------------------
	simdScore pawnResult;
	pawnEntry* probePawn= pawnHashTable.probe(getActualState().pawnKey);
	if(probePawn!=nullptr){
		pawnResult=simdScore{probePawn->res[0],probePawn->res[1],0,0};
		weakPawns=probePawn->weakPawns;
		passedPawns=probePawn->passedPawns;
		attackedSquares[whitePawns]=probePawn->pawnAttacks[0];
		attackedSquares[blackPawns]=probePawn->pawnAttacks[1];
		weakSquares[white]=probePawn->weakSquares[0];
		weakSquares[black]=probePawn->weakSquares[1];
		holes[white]=probePawn->holes[0];
		holes[black]=probePawn->holes[1];


	}
	else
	{


		pawnResult=simdScore{0,0,0,0};
		bitMap pawns= bitBoard[whitePawns];

		while(pawns){
			tSquare sq=firstOne(pawns);
			pawnResult+=evalPawn<white>(*this,sq, weakPawns, passedPawns);
			pawns &= pawns-1;
		}

		pawns= bitBoard[blackPawns];

		while(pawns){
			tSquare sq=firstOne(pawns);
			pawnResult-=evalPawn<black>(*this,sq, weakPawns, passedPawns);
			pawns &= pawns-1;
		}



		bitMap temp=bitBoard[whitePawns];
		bitMap pawnAttack=(temp & ~(FILEMASK[H1]))<<9;
		pawnAttack|=(temp & ~(FILEMASK[A1]))<<7;

		attackedSquares[whitePawns]=pawnAttack;
		pawnAttack|=pawnAttack<<8;
		pawnAttack|=pawnAttack<<16;
		pawnAttack|=pawnAttack<<32;

		weakSquares[white]=~pawnAttack;


		temp=bitBoard[blackPawns];
		pawnAttack=(temp & ~(FILEMASK[H1]))>>7;
		pawnAttack|=(temp & ~(FILEMASK[A1]))>>9;

		attackedSquares[blackPawns]=pawnAttack;

		pawnAttack|=pawnAttack>>8;
		pawnAttack|=pawnAttack>>16;
		pawnAttack|=pawnAttack>>32;

		weakSquares[black]=~pawnAttack;

		temp=bitBoard[whitePawns]<<8;
		temp|=temp<<8;
		temp|=temp<<16;
		temp|=temp<<32;

		holes[white]= weakSquares[white]&temp;



		temp=bitBoard[blackPawns]>>8;
		temp|=temp>>8;
		temp|=temp>>16;
		temp|=temp>>32;

		holes[black]= weakSquares[black]&temp;
		pawnResult-=(bitCnt(holes[white])-bitCnt(holes[black]))*simdScore{80,100,0,0};

		pawnHashTable.insert(st.pawnKey,pawnResult, weakPawns, passedPawns,attackedSquares[whitePawns],attackedSquares[blackPawns],weakSquares[white],weakSquares[black],holes[white],holes[black]);



	}

	res+=pawnResult;

	if(attackedSquares[whitePawns] & centerBitmap){
		res+=bitCnt(attackedSquares[whitePawns] & centerBitmap)*simdScore{100,0,0,0};
	}
	if(attackedSquares[whitePawns] & bigCenterBitmap){
		res+=bitCnt(attackedSquares[whitePawns] & bigCenterBitmap)*simdScore{50,0,0,0};
	}

	if(attackedSquares[blackPawns] & centerBitmap){
		res-=bitCnt(attackedSquares[blackPawns] & centerBitmap)*simdScore{100,0,0,0};
	}
	if(attackedSquares[blackPawns] & bigCenterBitmap){
		res-=bitCnt(attackedSquares[blackPawns] & bigCenterBitmap)*simdScore{50,0,0,0};
	}

	if(trace){
		sync_cout << std::setw(20) << "pawns" << " |   ---    --- |   ---    --- | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}





	//-----------------------------------------
	//	pieces
	//-----------------------------------------

	simdScore wScore;
	simdScore bScore;
	wScore=evalPieces<Position::whiteKnights>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	bScore=evalPieces<Position::blackKnights>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	res+=wScore-bScore;
	if(trace){
		sync_cout << std::setw(20) << "knights" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}

	wScore=evalPieces<Position::whiteBishops>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	bScore=evalPieces<Position::blackBishops>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	res+=wScore-bScore;
	if(trace){
		sync_cout << std::setw(20) << "bishops" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}

	wScore=evalPieces<Position::whiteRooks>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	bScore=evalPieces<Position::blackRooks>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	res+=wScore-bScore;
	if(trace){
		sync_cout << std::setw(20) << "rooks" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}

	wScore=evalPieces<Position::whiteQueens>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	bScore=evalPieces<Position::blackQueens>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	res+=wScore-bScore;
	if(trace){
		sync_cout << std::setw(20) << "queens" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}

	//todo valutazione pezzi

	//sync_cout<<"pieces:"<<res[0]<<":"<<res[1]<<sync_endl;
	attackedSquares[whiteKing]=Movegen::attackFromKing(pieceList[whiteKing][0]);
	attackedSquares[blackKing]=Movegen::attackFromKing(pieceList[blackKing][0]);

	attackedSquares[whitePieces]=attackedSquares[whiteKing]
								| attackedSquares[whiteKnights]
								| attackedSquares[whiteBishops]
								| attackedSquares[whiteRooks]
								| attackedSquares[whiteQueens]
								| attackedSquares[whitePawns];

	attackedSquares[blackPieces]=attackedSquares[blackKing]
								| attackedSquares[blackKnights]
								| attackedSquares[blackBishops]
								| attackedSquares[blackRooks]
								| attackedSquares[blackQueens]
								| attackedSquares[blackPawns];


	//-----------------------------------------
	//	passed pawn evalutation
	//-----------------------------------------
	// white passed pawns
	bitMap pp=passedPawns&bitBoard[whitePawns];

	wScore=simdScore{0,0,0,0};
	while(pp){
		simdScore passedPawnsBonus;
		tSquare ppSq=firstOne(pp);

		pp &= pp-1;

		unsigned int relativeRank=RANKS[ppSq];

		int r=relativeRank-1;
		int rr= r*(r-1);

		passedPawnsBonus=simdScore{600*rr,1100*(rr+r+1),0,0};

		if(rr){
			tSquare blockingSquare=ppSq+pawnPush(white);
			// bonus for king proximity to blocking square
			passedPawnsBonus+=simdScore{0,SQUARE_DISTANCE[blockingSquare][pieceList[blackKing][0]]*150*rr,0,0};
			passedPawnsBonus-=simdScore{0,SQUARE_DISTANCE[blockingSquare][pieceList[whiteKing][0]]*120*rr,0,0};

			if(squares[blockingSquare]==empty){
				bitMap forwardSquares=SQUARES_IN_FRONT_OF[0][ppSq];
				bitMap defendedSquares = forwardSquares & attackedSquares[whitePieces];

				bitMap unsafeSquares = forwardSquares & (attackedSquares[blackPieces] |bitBoard[blackPieces] );

				if(unsafeSquares){
					passedPawnsBonus-=simdScore{100*rr,100*rr,0,0};
					if(unsafeSquares & bitSet(blockingSquare)){
							passedPawnsBonus-=simdScore{100*rr,100*rr,0,0};
					}
				}
				if(defendedSquares==forwardSquares){
					passedPawnsBonus+=simdScore{100*rr,100*rr,0,0};
					if(defendedSquares & bitSet(blockingSquare)){
						passedPawnsBonus+=simdScore{20*rr,20*rr,0,0};
					}
				}
			}
		}

		if(FILES[ppSq]==0 || FILES[ppSq]==7){
			passedPawnsBonus -=passedPawnFileAHPenalty;
		}
		bitMap supportingPawns=bitBoard[whitePawns]&ISOLATED_PAWN[ppSq];
		if(supportingPawns & RANKMASK[ppSq]){
			passedPawnsBonus+=passedPawnSupportedBonus*r;
		}
		if(supportingPawns & RANKMASK[ppSq-pawnPush(0)]){
			passedPawnsBonus+=passedPawnSupportedBonus*(r/2);
		}

		wScore+=passedPawnsBonus;

	}

	// todo unsotppable passed pawn: ( more value to unstoppable passed pawn, less value or 0 for stobbable ones
	pp=passedPawns&bitBoard[blackPawns];

	bScore=simdScore{0,0,0,0};
	while(pp){
		simdScore passedPawnsBonus;
		tSquare ppSq=firstOne(pp);
		pp &= pp-1;

		unsigned int relativeRank=7-RANKS[ppSq];

		int r=relativeRank-1;
		int rr= r*(r-1);

		passedPawnsBonus=simdScore{600*rr,1100*(rr+r+1),0,0};

		if(rr){
			tSquare blockingSquare=ppSq+pawnPush(black);

			// bonus for king proximity to blocking square
			passedPawnsBonus+=simdScore{0,SQUARE_DISTANCE[blockingSquare][pieceList[whiteKing][0]]*150*rr,0,0};
			passedPawnsBonus-=simdScore{0,SQUARE_DISTANCE[blockingSquare][pieceList[blackKing][0]]*120*rr,0,0};

			if(squares[blockingSquare]==empty){
				bitMap forwardSquares=SQUARES_IN_FRONT_OF[1][ppSq];
				bitMap defendedSquares = forwardSquares & attackedSquares[blackPieces];

				bitMap unsafeSquares = forwardSquares & (attackedSquares[whitePieces] | bitBoard[whitePieces]);


				if(unsafeSquares){
					passedPawnsBonus-=simdScore{100*rr,100*rr,0,0};
					if(unsafeSquares & bitSet(blockingSquare)){
							passedPawnsBonus-=simdScore{100*rr,100*rr,0,0};
					}
				}
				if(defendedSquares==forwardSquares){
					passedPawnsBonus+=simdScore{100*rr,100*rr,0,0};
					if(defendedSquares & bitSet(blockingSquare)){
						passedPawnsBonus+=simdScore{20*rr,20*rr,0,0};
					}
				}
			}
		}

		if(FILES[ppSq]==0 || FILES[ppSq]==7){
			passedPawnsBonus -=passedPawnFileAHPenalty;
		}
		bitMap supportingPawns=bitBoard[blackPawns]&ISOLATED_PAWN[ppSq];
		if(supportingPawns & RANKMASK[ppSq]){
			passedPawnsBonus+=passedPawnSupportedBonus*r;
		}
		if(supportingPawns & RANKMASK[ppSq-pawnPush(1)]){
			passedPawnsBonus+=passedPawnSupportedBonus*(r/2);
		}

		bScore+=passedPawnsBonus;

	}
	res+=wScore-bScore;

	if(trace){
		sync_cout << std::setw(20) << "passed pawns" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}

	//sync_cout<<"passedPawns:"<<res[0]<<":"<<res[1]<<sync_endl;

	//todo attacked squares

	//---------------------------------------
	//	space
	//---------------------------------------
	// white pawns
	bitMap spacew =bitBoard[whitePawns];
	spacew|=spacew>>8;
	spacew|=spacew>>16;
	spacew|=spacew>>32;
	spacew &=~attackedSquares[blackPieces];
	//displayBitmap(spacew);



	// black pawns
	bitMap spaceb =bitBoard[blackPawns];
	spaceb|=spaceb<<8;
	spaceb|=spaceb<<16;
	spaceb|=spaceb<<32;
	spaceb &=~attackedSquares[whitePieces];

	//displayBitmap(spaceb);
	res+=(bitCnt(spacew)-bitCnt(spaceb))*simdScore{800,0,0,0};

	if(trace){
		wScore=bitCnt(spacew)*simdScore{800,0,0,0};
		bScore=bitCnt(spaceb)*simdScore{800,0,0,0};
		sync_cout << std::setw(20) << "space" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}



	//sync_cout<<"space:"<<res[0]<<":"<<res[1]<<sync_endl;

	//todo counterattack??

	//todo weakpawn

	//--------------------------------------
	//	weak pieces
	//--------------------------------------
	wScore=simdScore{0,0,0,0};
	bScore=simdScore{0,0,0,0};
	bitMap pawnAttackedPieces=bitBoard[whitePieces] & attackedSquares[blackPawns];
	while(pawnAttackedPieces){
		tSquare attacked=firstOne(pawnAttackedPieces);
		wScore-=attackedByPawnPenalty[squares[attacked]%separationBitmap];
		pawnAttackedPieces &=pawnAttackedPieces-1;
	}


	// todo fare un weak piece migliore:qualsiasi pezzo attaccato riceve un malus dipendente dal suo pi� debole attaccante e dal suo valore.
	// volendo anche da quale pezzo � difeso
	bitMap weakPieces=bitBoard[whitePieces] & attackedSquares[blackPieces] &~attackedSquares[whitePawns];
	bitMap undefendedMinors =  (bitBoard[whiteKnights] | bitBoard[whiteBishops])  & ~attackedSquares[whitePieces];
	if (undefendedMinors){
		wScore-=simdScore{956,254,0,0};
	}
	while(weakPieces){
		tSquare p=firstOne(weakPieces);

		bitboardIndex attackedPiece= squares[p];
		bitboardIndex attackingPiece=blackPawns;
		for(;attackingPiece>=separationBitmap;attackingPiece=(bitboardIndex)(attackingPiece-1)){
			if(attackedSquares[attackingPiece] & bitSet(p)){
				wScore-=weakPiecePenalty[attackedPiece%separationBitmap][attackingPiece%separationBitmap];
				break;
			}
		}
		weakPieces&=weakPieces-1;
	}





	pawnAttackedPieces=bitBoard[blackPieces] & attackedSquares[whitePawns];
	while(pawnAttackedPieces){
		tSquare attacked=firstOne(pawnAttackedPieces);
		bScore-=attackedByPawnPenalty[squares[attacked]%separationBitmap];
		pawnAttackedPieces &=pawnAttackedPieces-1;
	}
	weakPieces=bitBoard[blackPieces] & attackedSquares[whitePieces] &~attackedSquares[blackPawns];
	undefendedMinors =  (bitBoard[blackKnights] | bitBoard[blackBishops])  & ~attackedSquares[blackPieces];
	if (undefendedMinors){
		bScore-=simdScore{956,254,0,0};
	}
	while(weakPieces){
		tSquare p=firstOne(weakPieces);
		bitboardIndex attackedPiece= squares[p];
		bitboardIndex attackingPiece=whitePawns;
		for(;attackingPiece>=occupiedSquares;attackingPiece=(bitboardIndex)(attackingPiece-1)){
			if(attackedSquares[attackingPiece] & bitSet(p)){
				bScore-=weakPiecePenalty[attackedPiece%separationBitmap][attackingPiece%separationBitmap];
				break;
			}
		}
		weakPieces&=weakPieces-1;
	}
	res+=wScore-bScore;
	if(trace){
		sync_cout << std::setw(20) << "threat" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}


	//sync_cout<<"weakPieces:"<<res[0]<<":"<<res[1]<<sync_endl;




	//--------------------------------------
	//	king safety
	//--------------------------------------
	wScore=simdScore{0,0,0,0};
	bScore=simdScore{0,0,0,0};
	int lwScore =0;
	int lbScore =0;
	int kingSafety[2]={0,0};

	tSquare ksq=pieceList[whiteKing][0];
	bitMap pawnShield=kingShield[white]&bitBoard[whitePawns];
	bitMap pawnFarShield=kingFarShield[white]&bitBoard[whitePawns];
	bitMap pawnStorm=PASSED_PAWN[white][ksq]&bitBoard[blackPawns];
	if(pawnShield){
		kingSafety[0]+=bitCnt(pawnShield)*kingShieldBonus;
	}
	if(pawnFarShield){
		kingSafety[0]+=bitCnt(pawnFarShield)*kingFarShieldBonus;
	}

	while(pawnStorm){
		tSquare p=firstOne(pawnStorm);
		kingSafety[0]-=(8-SQUARE_DISTANCE[p][ksq])*kingStormBonus;
		pawnStorm&= pawnStorm-1;
	}


	if((st.castleRights & wCastleOO)
		&& !(attackedSquares[blackPieces] & (bitSet(E1)|bitSet(F1)|bitSet(G1) ))
		&& !(bitBoard[occupiedSquares] & (bitSet(F1)|bitSet(G1)))
		){
		int ks=0;
		bitMap localKingRing=Movegen::attackFromKing(G1);
		bitMap localKingShield=localKingRing;
		localKingRing|=Movegen::attackFromKing(G2);
		bitMap localKingFarShield=localKingRing&~(localKingShield);

		pawnShield=localKingShield&bitBoard[whitePawns];
		pawnFarShield=localKingFarShield&bitBoard[whitePawns];
		pawnStorm=PASSED_PAWN[white][G1]&bitBoard[blackPawns];
		if(pawnShield){
			ks=bitCnt(pawnShield)*kingShieldBonus;
		}
		if(pawnFarShield){
			ks+=bitCnt(pawnFarShield)*kingFarShieldBonus;
		}
		while(pawnStorm){
			tSquare p=firstOne(pawnStorm);
			ks-=(8-SQUARE_DISTANCE[p][G1])*kingStormBonus;
			pawnStorm&= pawnStorm-1;
		}
		kingSafety[0]=std::max(ks,kingSafety[0]);
	}

	if((st.castleRights & wCastleOOO)
		&& !(attackedSquares[blackPieces] & (bitSet(E1)|bitSet(D1)|bitSet(C1) ))
		&& !(bitBoard[occupiedSquares] & (bitSet(D1)|bitSet(C1)|bitSet(B1)))
		){
		int ks=0;
		bitMap localKingRing=Movegen::attackFromKing(C1);
		bitMap localKingShield=localKingRing;
		localKingRing|=Movegen::attackFromKing(C2);
		bitMap localKingFarShield=localKingRing&~(localKingShield);

		pawnShield=localKingShield&bitBoard[whitePawns];
		pawnFarShield=localKingFarShield&bitBoard[whitePawns];
		pawnStorm=PASSED_PAWN[white][C1]&bitBoard[blackPawns];

		if(pawnShield){
			ks=bitCnt(pawnShield)*kingShieldBonus;
		}
		if(pawnFarShield){
			ks+=bitCnt(pawnFarShield)*kingFarShieldBonus;
		}
		while(pawnStorm){
			tSquare p=firstOne(pawnStorm);
			ks-=(8-SQUARE_DISTANCE[p][C1])*kingStormBonus;
			pawnStorm&= pawnStorm-1;
		}
		kingSafety[0]=std::max(ks,kingSafety[0]);
	}
	lwScore=kingSafety[0];


	ksq=pieceList[blackKing][0];

	pawnShield=kingShield[black]&bitBoard[blackPawns];
	pawnFarShield=kingFarShield[black]&bitBoard[blackPawns];
	pawnStorm=PASSED_PAWN[black][ksq]&bitBoard[whitePawns];
	if(pawnShield){
		kingSafety[1]+=bitCnt(pawnShield)*kingShieldBonus;
	}
	if(pawnFarShield){
		kingSafety[1]+=bitCnt(pawnFarShield)*kingFarShieldBonus;
	}
	while(pawnStorm){
		tSquare p=firstOne(pawnStorm);
		kingSafety[1]-=(8-SQUARE_DISTANCE[p][ksq])*kingStormBonus;
		pawnStorm&= pawnStorm-1;
	}

	if((st.castleRights & bCastleOO)
		&& !(attackedSquares[whitePieces] & (bitSet(E8)|bitSet(F8)|bitSet(G8) ))
		&& !(bitBoard[occupiedSquares] & (bitSet(F8)|bitSet(G8)))
		){
		int ks=0;
		bitMap localKingRing=Movegen::attackFromKing(G8);
		bitMap localKingShield=localKingRing;
		localKingRing|=Movegen::attackFromKing(G7);
		bitMap localKingFarShield=localKingRing&~(localKingShield);

		pawnShield=localKingShield&bitBoard[blackPawns];
		pawnFarShield=localKingFarShield&bitBoard[blackPawns];
		pawnStorm=PASSED_PAWN[black][G8]&bitBoard[whitePawns];
		if(pawnShield){
			ks=bitCnt(pawnShield)*kingShieldBonus;
		}
		if(pawnFarShield){
			ks+=bitCnt(pawnFarShield)*kingFarShieldBonus;
		}
		while(pawnStorm){
			tSquare p=firstOne(pawnStorm);
			ks-=(8-SQUARE_DISTANCE[p][G8])*kingStormBonus;
			pawnStorm&= pawnStorm-1;
		}
		kingSafety[1]=std::max(ks,kingSafety[1]);
	}

	if((st.castleRights & bCastleOOO)
		&& !(attackedSquares[whitePieces] & (bitSet(E8)|bitSet(D8)|bitSet(C8) ))
		&& !(bitBoard[occupiedSquares] & (bitSet(D8)|bitSet(C8)|bitSet(B8)))
		){
		int ks=0;
		bitMap localKingRing=Movegen::attackFromKing(C8);
		bitMap localKingShield=localKingRing;
		localKingRing|=Movegen::attackFromKing(C7);
		bitMap localKingFarShield=localKingRing&~(localKingShield);

		pawnShield=localKingShield&bitBoard[blackPawns];
		pawnFarShield=localKingFarShield&bitBoard[blackPawns];
		pawnStorm=PASSED_PAWN[black][C8]&bitBoard[whitePawns];
		if(pawnShield){
			ks=bitCnt(pawnShield)*kingShieldBonus;
		}
		if(pawnFarShield){
			ks+=bitCnt(pawnFarShield)*kingFarShieldBonus;
		}
		while(pawnStorm){
			tSquare p=firstOne(pawnStorm);
			ks-=(8-SQUARE_DISTANCE[p][C8])*kingStormBonus;
			pawnStorm&= pawnStorm-1;
		}
		kingSafety[1]=std::max(ks,kingSafety[1]);
	}
	lbScore=kingSafety[1];

	res+=kingSafety[0]-kingSafety[1];



	if(kingAttackersCount[white]>=2 && kingAdjacentZoneAttacksCount[white])
	{
		res+=kingSafety[0];

		bitMap undefendedSquares=
			attackedSquares[whitePieces]& attackedSquares[blackKing];
		undefendedSquares&=
			~(attackedSquares[blackPawns]
			|attackedSquares[blackKnights]
			|attackedSquares[blackBishops]
			|attackedSquares[blackRooks]
			|attackedSquares[blackQueens]);

		signed int attackUnits =  std::min((unsigned int)25, (kingAttackersCount[white] * kingAttackersWeight[white]) / 2)
		                     + 3 * (kingAdjacentZoneAttacksCount[white] + bitCnt(undefendedSquares))
		                     + KingExposed[63-pieceList[blackKing][0]]
		                     - kingSafety[0] / 500;


		// safe contact queen check
		bitMap safeContactSquare=undefendedSquares & attackedSquares[whiteQueens];
		safeContactSquare &= (attackedSquares[whiteRooks]| attackedSquares[whiteBishops] | attackedSquares[whiteKnights]| attackedSquares[whitePawns]);

		if(safeContactSquare){
			attackUnits+=5*bitCnt(safeContactSquare);
		}

		// safe contact rook check
		safeContactSquare=undefendedSquares & attackedSquares[whiteRooks];
		safeContactSquare &= (attackedSquares[whiteRooks]| attackedSquares[whiteBishops] | attackedSquares[whiteKnights]| attackedSquares[whitePawns]);

		safeContactSquare &=Movegen::getRookPseudoAttack(pieceList[blackKing][0]);

		if(safeContactSquare){
			attackUnits+=3*bitCnt(safeContactSquare);
		}

		// long distance check
		bitMap rMap=Movegen::attackFromRook(pieceList[blackKing][0],bitBoard[occupiedSquares]);
		bitMap bMap=Movegen::attackFromBishop(pieceList[blackKing][0],bitBoard[occupiedSquares]);

		// vertical check
		bitMap longDistCheck=rMap & (attackedSquares[whiteRooks]| attackedSquares[whiteQueens]) & ~attackedSquares[blackPieces] & ~bitBoard[whitePieces];
		if(longDistCheck){
			attackUnits+=3*bitCnt(longDistCheck);
		}

		// diagonal check
		longDistCheck=bMap & (attackedSquares[whiteBishops]| attackedSquares[whiteQueens]) & ~attackedSquares[blackPieces] & ~bitBoard[whitePieces];
		if(longDistCheck){
			attackUnits+=2*bitCnt(longDistCheck);
		}

		///knight check;
		longDistCheck=Movegen::attackFromKnight(pieceList[blackKing][0]) & (attackedSquares[whiteKnights]) & ~attackedSquares[blackPieces] & ~bitBoard[whitePieces];
		if(longDistCheck){
			attackUnits+=bitCnt(longDistCheck);
		}
		attackUnits = std::min(99, std::max(0, attackUnits));
		attackUnits*=attackUnits;
		res+=simdScore{attackUnits*3,attackUnits/5,0,0};
		if(trace){
			wScore +=simdScore{attackUnits*3,attackUnits/5,0,0};
			wScore += simdScore{ lwScore,0,0,0};
		}

	}


	if(kingAttackersCount[black]>=2 && kingAdjacentZoneAttacksCount[black])
	{

		res-=kingSafety[1];
		bitMap undefendedSquares=
			attackedSquares[blackPieces] & attackedSquares[whiteKing];
		undefendedSquares&=
			~(attackedSquares[whitePawns]
			|attackedSquares[whiteKnights]
			|attackedSquares[whiteBishops]
			|attackedSquares[whiteRooks]
			|attackedSquares[whiteQueens]);

		signed int attackUnits =  std::min((unsigned int)25, (kingAttackersCount[black] * kingAttackersWeight[black]) / 2)
							 + 3 * (kingAdjacentZoneAttacksCount[black] + bitCnt(undefendedSquares))
							 + KingExposed[pieceList[whiteKing][0]]
							 - kingSafety[1] / 500;

		// safe contact queen check
		bitMap safeContactSquare=undefendedSquares & attackedSquares[blackQueens];
		safeContactSquare &= (attackedSquares[blackRooks]| attackedSquares[blackBishops] | attackedSquares[blackKnights]| attackedSquares[blackPawns]);

		if(safeContactSquare){
			attackUnits+=5*bitCnt(safeContactSquare);
		}
		// safe contact rook check

		safeContactSquare=undefendedSquares & attackedSquares[blackRooks];
		safeContactSquare &= (attackedSquares[blackRooks]| attackedSquares[blackBishops] | attackedSquares[blackKnights]| attackedSquares[blackPawns]);

		safeContactSquare &=Movegen::getRookPseudoAttack(pieceList[whiteKing][0]);

		if(safeContactSquare){
			attackUnits+=3*bitCnt(safeContactSquare);
		}

		// long distance check
		bitMap rMap=Movegen::attackFromRook(pieceList[whiteKing][0],bitBoard[occupiedSquares]);
		bitMap bMap=Movegen::attackFromBishop(pieceList[whiteKing][0],bitBoard[occupiedSquares]);

		// vertical check
		bitMap longDistCheck=rMap & (attackedSquares[blackRooks]| attackedSquares[blackQueens]) & ~attackedSquares[whitePieces] & ~bitBoard[blackPieces];
		if(longDistCheck){
			attackUnits+=3*bitCnt(longDistCheck);
		}

		// diagonal check
		longDistCheck=bMap & (attackedSquares[blackBishops]| attackedSquares[blackQueens]) & ~attackedSquares[whitePieces] & ~bitBoard[blackPieces];
		if(longDistCheck){
			attackUnits+=2*bitCnt(longDistCheck);
		}

		///knight check;
		longDistCheck=Movegen::attackFromKnight(pieceList[whiteKing][0]) & (attackedSquares[blackKnights]) & ~attackedSquares[whitePieces] & ~bitBoard[blackPieces];
		if(longDistCheck){
			attackUnits+=bitCnt(longDistCheck);
		}


		attackUnits = std::min(99, std::max(0, attackUnits));
		attackUnits*=attackUnits;
		res-=simdScore{attackUnits*3,attackUnits/5,0,0};
		if(trace){
			bScore +=simdScore{ attackUnits*3,attackUnits/5,0,0};
			bScore += simdScore{ lbScore,0,0,0};
		}


	}

	if(trace){
		sync_cout << std::setw(20) << "king safety" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}


	//sync_cout<<"kingSafety:"<<res[0]<<":"<<res[1]<<sync_endl;
	//todo scaling



	//--------------------------------------
	//	finalizing
	//--------------------------------------
	signed int gamePhase=getGamePhase();

	signed long long r=((signed long long)res[0])*(65536-gamePhase)+((signed long long)res[1])*gamePhase;

	Score score =(r)/65536;
	if(mulCoeff!=256){
		score*=mulCoeff;
		score/=256;
	}

	// final value saturation
	score=std::min(highSat,score);
	score=std::max(lowSat,score);

	evalTable.insert(st.pawnKey,score);


	if(st.nextMove)
	{
		return -score;
	}
	else{
		return score;
	}

}

template Score Position::eval<false>(pawnTable& pawnHashTable, evalTable& evalTable) const;
template Score Position::eval<true>(pawnTable& pawnHashTable, evalTable& evalTable) const;
