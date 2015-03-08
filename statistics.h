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

#ifndef STATISTICS_H_
#define STATISTICS_H_

#include "vajolet.h"
#include "io.h"
#include "position.h"
#include "search.h"


class Statistics{
public :
	static Statistics& instance(){
	  static Statistics instance;
	  return instance;
	}

	unsigned long long QSPVnodeCounter;
	unsigned long long QSPVnodeOrderingTThit;
	unsigned long long QSPVnodeOrderingAcc;
	unsigned long long QSPVnodeOrdering[40];

	unsigned long long PVnodeCounter[30];
	unsigned long long PVnodeOrderingTThit[30];
	unsigned long long PVnodeOrderingAcc[30];
	unsigned long long PVnodeOrdering[30][80];



	void printNodeTypeStat();
	void initNodeTypeStat();
private:
	Statistics(){}
};



#endif /* STATISTICS_H_ */
