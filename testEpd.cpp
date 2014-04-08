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
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <vector>
#include <iomanip>
#include <cmath>
#include "testEpd.h"
#include "eval.h"
#include "position.h"
#include "search.h"
#include "transposition.h"

int testEpd(std::string epdFile){
	std::ifstream ifs (epdFile, std::ifstream::in);
	if ( ifs.fail()){
		sync_cout << "epd opening error"<<sync_endl;
		return 0;
	}
	char cline[256];
	do{
		ifs.getline(cline,256);
		std::string line=cline;
//		sync_cout<<line<<sync_endl;
		if(line.length()!=0){
			// find bm or am
			int amPos=line.find(" am ");
			int bmPos=line.find(" bm ");
			int separator=-1;

			if(amPos!=-1){
				separator=amPos;
				//sync_cout<<"found am"<<sync_endl;
			}
			if(bmPos!=-1){
				separator=bmPos;
				//sync_cout<<"found bm"<<sync_endl;
			}

			std::string Fen=line.substr(0,separator);
			sync_cout<<Fen<<sync_endl;
			int separator2=line.find(";");
			std::string Move= line.substr(separator+4,separator2-separator-4);
			sync_cout<<Move<<sync_endl;



		}
	}while(ifs.good());

	return 1;
}



