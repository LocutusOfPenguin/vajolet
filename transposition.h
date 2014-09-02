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
#ifndef TRANSPOSITION_H_
#define TRANSPOSITION_H_

#include "vajolet.h"
#include <stdlib.h>
#include <cstring>





enum ttType{
	typeExact,
	typeScoreLowerThanAlpha,
	typeScoreHigherThanBeta
};

struct ttEntry{
private :
	unsigned int key; 			/*! 32 bit for the upper part of the key*/
	Score value;				/*! 32 bit for the value*/
	Score staticValue;			/*! 32 bit for the static evalutation (eval())*/
	unsigned short packedMove;	/*!	16 bit for the move*/
	signed short int depth;		/*! 16 bit for depth*/
	unsigned char searchId;		/*! 8 bit for the generation id*/
	unsigned char type;			/*! 8 bit for the type of the entry*/
								/*  144 bits total =18 bytes*/
public:
	void save(unsigned int k,Score val,unsigned char t,signed short int d,unsigned short m,Score sv){
		assert(val<SCORE_INFINITE);
		assert(val>-SCORE_INFINITE);
		assert(sv<SCORE_INFINITE);
		assert(sv>-SCORE_INFINITE);
		assert(type<=2);
		key=k;
		value=val;
		staticValue=sv;
		packedMove= m;
		depth=d;
		type=t;

	}
	void setGeneration(unsigned char gen){
		searchId=gen;
	}

	inline unsigned int getKey() const{return key;}
	Score getValue()const {return value;}
	Score getStaticValue()const {return staticValue;}
	unsigned short getPackedMove()const {return packedMove;}
	signed short int getDepth()const {return depth;}
	unsigned char getSearchId()const {return searchId;}
	unsigned char getType()const {return type;}
	unsigned char getGeneration()const {return searchId;}


};

struct ttCluster{
	ttEntry data[4];
};


class transpositionTable{
	ttCluster* table;
	unsigned int elements;
	unsigned int usedElements;
	unsigned char generation;
public:
	transpositionTable(){
		generation=0;
		elements=1;
		usedElements=0;
	}
	~transpositionTable(){
		if(table){
			free(table);
		}
	}
	void newSearch() { generation++; usedElements=0; }
	void setSize(unsigned int mbSize);
	void clear();

	inline ttCluster* findCluster(U64 key) const {
		return table + (((unsigned int)key) % elements);
	}

	inline void refresh(const ttEntry* tte) {
		assert(tte!=nullptr);
		if(const_cast<ttEntry*>(tte)->getSearchId()!=generation){
			usedElements++;
		}
		const_cast<ttEntry*>(tte)->setGeneration(generation);

	}

	ttEntry* probe(const U64 key) const;
	void store(const U64 key, Score v, unsigned char b, signed short int d, unsigned short m, Score statV);

	unsigned int getFullness(void){
		unsigned int ret = usedElements*250.0/(elements);
		return ret;
	}

	// value_to_tt() adjusts a mate score from "plies to mate from the root" to
	// "plies to mate from the current position". Non-mate scores are unchanged.
	// The function is called before storing a value to the transposition table.

	static Score scoreToTT(Score v, int ply){
		assert(v<=SCORE_MATE);
		assert(v>=SCORE_MATED);
		assert(ply>=0);
		assert(v+ply<=SCORE_MATE);
		assert(v-ply>=SCORE_MATED);
		return  v >= SCORE_MATE_IN_MAX_PLY  ? v + ply
				: v <= SCORE_MATED_IN_MAX_PLY ? v - ply : v;
	}


	// value_from_tt() is the inverse of value_to_tt(): It adjusts a mate score
	// from the transposition table (where refers to the plies to mate/be mated
	// from current position) to "plies to mate/be mated from the root".

	static Score scoreFromTT(Score v, int ply){
		assert(v!=SCORE_NONE);
		assert(ply>=0);
		assert(v-ply<SCORE_MATE);
		assert(v+ply>SCORE_MATED);
		assert(v>=SCORE_MATED);
		assert(v<=SCORE_MATE);
		return  v == SCORE_NONE ? SCORE_NONE
				: v >= SCORE_MATE_IN_MAX_PLY  ? v - ply
				: v <= SCORE_MATED_IN_MAX_PLY ? v + ply : v;
	}
};

extern transpositionTable TT;




#endif /* TRANSPOSITION_H_ */
