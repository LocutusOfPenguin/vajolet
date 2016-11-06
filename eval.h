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



#ifndef EVAL_H_
#define EVAL_H_

#include "vajolet.h"
#include "position.h"
#include "tables.h"


void initMobilityBonus();

extern simdScore knightOnOutpost;
extern simdScore knightOnOutpostSupported;
extern simdScore knightOnHole;
extern simdScore KnightAttackingWeakPawn;
extern simdScore bishopOnOutpost;
extern simdScore bishopOnOutpostSupported;
extern simdScore bishopOnHole;
extern simdScore badBishop;





#endif /* EVAL_H_ */
