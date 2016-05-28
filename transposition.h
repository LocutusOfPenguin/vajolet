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





enum ttType
{
	typeExact,
	typeScoreLowerThanAlpha,
	typeScoreHigherThanBeta
};

class ttEntry
{
public:

	Score value;				/*! 32 bit for the value*/
	Score staticValue;			/*! 32 bit for the static evalutation (eval())*/

	struct _bit
	{
		unsigned int packedMove:16;	/*!	16 bit for the move*/
		unsigned int key:16;		/*! 32 bit for the upper part of the key*/
	}bit;
	struct _bit2{
		signed short int depth:12;/*! 12 bit for depth*/
		unsigned char generation:2;	/*! 2 bit for the generation id*/
		unsigned char type:2;		/*! 2 bit for the type of the entry*/
	}bit2;


								/*  144 bits total =18 bytes*/
public:
	void save(unsigned int Key, Score Value, unsigned char Type, signed short int Depth, unsigned short Move, Score StaticValue)
	{
		assert(Value < SCORE_INFINITE || Value == SCORE_NONE);
		assert(Value >- SCORE_INFINITE);
		assert(StaticValue < SCORE_INFINITE);
		assert(StaticValue > -SCORE_INFINITE);
		assert(Type <= typeScoreHigherThanBeta);
		bit.key = Key;
		value = Value;
		staticValue = StaticValue;
		bit.packedMove = Move;
		bit2.depth = Depth;
		bit2.type = Type;
	}

	void setGeneration(unsigned char gen)
	{
		bit2.generation = gen;
	}

	inline unsigned int getKey() const{ return bit.key; }
	Score getValue()const { return value; }
	Score getStaticValue()const { return staticValue; }
	unsigned short getPackedMove()const { return bit.packedMove; }
	signed short int getDepth()const {return bit2.depth; }
	unsigned char getType()const { return bit2.type; }
	unsigned char getGeneration()const { return bit2.generation; }


};

struct ttCluster
{
	ttEntry data[4];
};


class transpositionTable
{
private:
	ttCluster* table;
	unsigned long int elements;
	unsigned long int usedElements;
	unsigned char generation;

public:
	transpositionTable()
	{
		table = nullptr;
		generation = 0;
		elements = 1;
		usedElements = 0;
	}
	~transpositionTable()
	{
		if(table)
		{
			free(table);
		}
	}

	void newSearch() { generation++; usedElements=0; }
	void setSize(unsigned long int mbSize);
	void clear();

	inline ttCluster& findCluster(U64 key) const
	{
		return table[((unsigned int)key) % elements];
	}

	inline void refresh(ttEntry* const tte)
	{
		assert(tte != nullptr);

		if(tte->getGeneration() != generation)
		{
			usedElements++;
			tte->setGeneration(generation);
		}


	}

	ttEntry* probe(const U64 key) const;

	void store(const U64 key, Score value, unsigned char type, signed short int depth, unsigned short move, Score statValue);

	unsigned int getFullness(void)
	{
		unsigned int ret = (unsigned int)(((unsigned long long int)usedElements*250)/elements);
		return ret;
	}

	// value_to_tt() adjusts a mate score from "plies to mate from the root" to
	// "plies to mate from the current position". Non-mate scores are unchanged.
	// The function is called before storing a value to the transposition table.

	static Score scoreToTT(Score v, int ply)
	{
		assert(v<=SCORE_MATE);
		assert(v>=SCORE_MATED);
		assert(ply>=0);
		assert(v+ply<=SCORE_MATE);
		assert(v-ply>=SCORE_MATED);
		assert(v != SCORE_NONE);
		return  v >= SCORE_MATE_IN_MAX_PLY  ? v + ply
				: v <= SCORE_MATED_IN_MAX_PLY ? v - ply : v;
	}


	// value_from_tt() is the inverse of value_to_tt(): It adjusts a mate score
	// from the transposition table (where refers to the plies to mate/be mated
	// from current position) to "plies to mate/be mated from the root".

	static Score scoreFromTT(Score v, int ply)
	{

		assert(ply>=0);
		assert(v-ply<SCORE_MATE || v == SCORE_NONE);
		assert(v+ply>SCORE_MATED);
		assert(v>=SCORE_MATED);
		assert(v<=SCORE_MATE || v == SCORE_NONE);
		return  v == SCORE_NONE ? SCORE_NONE
				: v >= SCORE_MATE_IN_MAX_PLY  ? v - ply
				: v <= SCORE_MATED_IN_MAX_PLY ? v + ply : v;
	}
};

extern transpositionTable TT;




#endif /* TRANSPOSITION_H_ */
