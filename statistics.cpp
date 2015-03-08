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

#include "statistics.h"

void Statistics::printNodeTypeStat(){
	unsigned long long totalCount =0;
	sync_cout<<"node ordering stats"<<std::endl<<sync_endl;


	for(int x=0;x<30;x++)
	{
		int lastValue=0;
		sync_cout<<"--------depth "<<x<<"-------------------------------------"<<sync_endl;
		sync_cout<<"PVNodes "<<PVnodeCounter[x]<<sync_endl;

		sync_cout<<"avg "<<double(PVnodeOrderingAcc[x])/PVnodeCounter[x]<<sync_endl;
		sync_cout<<"TThit "<<PVnodeOrderingTThit[x]<<sync_endl;
		for(int i=0;i<80;i++){
			if(PVnodeOrdering[x][i]!=0)
			{
				lastValue=i;
			}
		}
		for(int i=0;i<=lastValue;i++)
		{
			totalCount+=PVnodeOrdering[x][i];
			sync_cout<<" "<<i<<" "<<PVnodeOrdering[x][i]<<" "<<double(PVnodeOrdering[x][i])*100.0/PVnodeCounter[x]<<"%"<<sync_endl;
		}
	}
	sync_cout<<"Total "<<totalCount<<sync_endl;


	sync_cout<<"QSPVNodes "<<QSPVnodeCounter<<sync_endl;
	sync_cout<<"QSstandPatNodes "<<double(QSPVnodeOrderingAcc)/QSPVnodeCounter<<sync_endl;

	sync_cout<<"TT "<<QSPVnodeOrderingTThit<<sync_endl;
	for(int i=0;i<20;i++)
	{
		sync_cout<<i<<" "<<QSPVnodeOrdering[i]<<" "<<double(QSPVnodeOrdering[i])*100.0/QSPVnodeCounter<<"%"<<sync_endl;
	}





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


