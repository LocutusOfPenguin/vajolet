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

#include "transposition.h"


transpositionTable TT;

void transpositionTable::setSize(unsigned long int mbSize)
{
	long long unsigned int size = (long unsigned int)( ((unsigned long long int)mbSize << 20) / sizeof(ttCluster));
	elements = size;
	if(table)
	{
		free(table);
	}
	table = (ttCluster*)calloc(elements,sizeof(ttCluster));
	if (table == nullptr)
	{
		std::cerr << "Failed to allocate " << mbSize<< "MB for transposition table." << std::endl;
		exit(EXIT_FAILURE);
	}
}

void transpositionTable::clear()
{
	std::memset(table, 0, elements * sizeof(ttCluster));
}

ttEntry* transpositionTable::probe(const U64 key) const
{

	ttCluster& ttc = findCluster(key);
	unsigned short keyH = (unsigned int)(key >> 32);


	for (unsigned i = 0; i < 4; i++)
	{
		if ( ttc.data[i].getKey() == keyH )
			return &(ttc.data[i]);
	}

	return nullptr;
}

void transpositionTable::store(const U64 key, Score value, unsigned char type, signed short int depth, unsigned short move, Score statValue)
{

	int c1, c2, c3;
	ttEntry *tte, *replace;
	unsigned short key32 = (unsigned int)(key >> 32); // Use the high 32 bits as key inside the cluster

	ttCluster& ttc = findCluster(key);
	tte = replace = ttc.data;

	assert(tte!=nullptr);
	assert(replace!=nullptr);

	for (unsigned i = 0; i < 4; i++, tte++)
	{
		if(!tte->getKey() || tte->getKey() == key32) // Empty or overwrite old
		{

			refresh(tte);
			if(!move){move = tte->getPackedMove();}
			tte->save(key32, value, type, depth, move, statValue);
			return;
		}


		// Implement replace strategy
		c1 = (replace->getGeneration() == generation ?  2 : 0);
		c2 = (tte->getGeneration() == generation || tte->getType() == typeExact ? -2 : 0);
		c3 = (tte->getDepth() < replace->getDepth() ?  1 : 0);

		if (c1 + c2 + c3 > 0)
		{
			replace = tte;
		}
	}

	assert(replace!=nullptr);

	refresh(replace);
	replace->save(key32, value, type, depth, move, statValue);




}
