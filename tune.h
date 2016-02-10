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

#ifndef TUNE_H_
#define TUNE_H_

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "position.h"
#include "io.h"
#include "vajolet.h"


extern simdScore initialPieceValue[Position::lastBitboard];

class Tuner
{



	typedef struct sParameterStruct{
		simdScore * parameter;
		//unsigned int parameter;
		unsigned int index;
		std::string name;
		signed int delta;
		sParameterStruct(std::string n,simdScore * p,unsigned int in,signed int d){
			parameter=p;
			index=in;
			name=n;
			delta=d;
		};
	}parameterStruct;

	void showValues(const std::vector<parameterStruct>& parameters);

	std::string epdFile;
	double scaling = 15000.0;
public:
	Tuner():epdFile("positions.epd"){}
	double parseEpd();
	void FindBestScaling();
	void tuneParameters();


};




#endif /* TUNE_H_ */
