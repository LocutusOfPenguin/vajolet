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

#include <fstream>
#include <iostream>
#include "statistics.h"

void Statistics::printNodeTypeStat(){
	unsigned long long totalCount =0;

	std::ofstream fs(".\\out.txt",std::ofstream::out | std::ofstream::app);
	fs<<"node ordering stats"<<std::endl<<sync_endl;


	for(int x=0;x<30;x++)
	{
		int lastValue=-1;
		for(int i=0;i<80;i++){
			if(PVnodeOrdering[x][i]!=0)
			{
				lastValue=i;
			}
		}
		if(lastValue!=-1)
		{
			fs<<"--------depth "<<x<<"-------------------------------------"<<std::endl;
			fs<<"PVNodes "<<PVnodeCounter[x]<<std::endl;

			fs<<"avg "<<double(PVnodeOrderingAcc[x])/PVnodeCounter[x]<<std::endl;
			fs<<"TThit "<<PVnodeOrderingTThit[x]<<std::endl;


			for(int i=0;i<=lastValue;i++)
			{
				totalCount+=PVnodeOrdering[x][i];
				fs<<" "<<i<<" "<<PVnodeOrdering[x][i]<<" "<<double(PVnodeOrdering[x][i])*100.0/PVnodeCounter[x]<<"%"<<std::endl;
			}
		}
	}
	fs<<"Total "<<totalCount<<std::endl;


	fs<<"QSPVNodes "<<QSPVnodeCounter<<std::endl;
	fs<<"QSstandPatNodes "<<double(QSPVnodeOrderingAcc)/QSPVnodeCounter<<std::endl;

	fs<<"TThit "<<QSPVnodeOrderingTThit<<std::endl;
	for(int i=0;i<20;i++)
	{
		fs<<i<<" "<<QSPVnodeOrdering[i]<<" "<<double(QSPVnodeOrdering[i])*100.0/QSPVnodeCounter<<"%"<<std::endl;
	}

	fs.close();





}



void Statistics::initNodeTypeStat(){


	for(int x=0;x<30;x++)
	{
		PVnodeOrderingTThit[x] =0;
		PVnodeOrderingAcc[x]=0;
		PVnodeCounter[x] = 0;
		for(int i=0;i<80;i++)
		{
			PVnodeOrdering[x][i]=0;
		}
	}

	QSPVnodeOrderingTThit =0;
	QSPVnodeOrderingAcc=0;
	QSPVnodeCounter = 0;
	for(int i=0;i<40;i++)
	{
		QSPVnodeOrdering[i]=0;
	}
}


