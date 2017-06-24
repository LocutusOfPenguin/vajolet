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

#ifndef MOVE_H_
#define MOVE_H_

#include "vajolet.h"

/*!	\brief struct move
    \author Marco Belli
	\version 1.0
	\date 08/11/2013
 */
class Move
{
public:
	Move(){}
	Move(const Move& m): packed(m.packed){}
	Move(const Move&& m):packed(m.packed){}
	Move(unsigned short i):packed(i){}
	union
	{
		struct
		{
			unsigned from		:6;
			unsigned to			:6;
			unsigned promotion	:2;
			unsigned flags		:2;
		}bit;
		unsigned short packed;
	};
	enum eflags{
		fnone,
		fpromotion,
		fenpassant,
		fcastle,
	};

	enum epromotion{
		promQueen,
		promRook,
		promBishop,
		promKnight,
	};


	inline bool operator == (const Move& d1) const { return packed == d1.packed;}
	inline bool operator != (const Move& d1) const { return packed != d1.packed;}
	inline Move& operator = (unsigned short b) { packed = b; return *this;}
	inline Move& operator = (const Move&m){ packed = m.packed; return *this;}
	inline Move& operator = (const Move&&m){ packed = m.packed; return *this;}
	inline Move( Move && m){packed = m.packed;};



};

class extMove : public Move
{
public:
	inline bool operator != (const Move& d1) const { return packed != d1.packed;}
	inline Move& operator = (const Move&m){ packed = m.packed; return *this;}
	Score score;

	extMove(){};
};

/*!	\brief return the offset of a pawn push
    \author Marco Belli
	\version 1.0
	\date 08/11/2013
 */
inline tSquare pawnPush(int color){
	return color? sud:north;
}





#endif /* MOVE_H_ */
