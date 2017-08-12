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
#include "bitops.h"
#include "bitmap.h"




/*! \brief display a bitmap on stdout
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
std::string bitmap2::displayBitmap() const
{
	std::string s;

	for (int rank = 7 ; rank >= 0; rank--)
	{
		s += std::to_string(rank + 1) + " ";
		for (int file = 0 ; file <= 7; file++)
		{

			if ( isSquareInBitmap( b, file, rank ) )
			{
				s += '1';
			}
			else
			{
				s += '.';
			}

		}
		s += "\n";
	}
	s +=  "  abcdefgh";
	return (s);
}


