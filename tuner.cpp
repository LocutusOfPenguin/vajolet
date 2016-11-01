/*
 * tuner.cpp
 *
 *  Created on: 01 nov 2016
 *      Author: elcab_000
 */

#include <iomanip>
#include <fstream>
#include "io.h"
#include "position.h"
#include "search.h"
#include "command.h"
#include "eval.h"

double sigmoid(Score input, double scaling )
{
	return 1.0/(1.0+ std::pow(10.0,-(input * scaling)/400.0));
}

double calcError()
{
	double scaling = 0.011;
	double totalError = 0.0;
	unsigned long long count = 0;
	Search src;
	std::string delimiter = ";";
	std::ifstream input("positions.epd");
	for (std::string line; getline(input, line);) {
		int stringPos = line.find(delimiter);
		std::string fen = line.substr(0, stringPos);
		double gameRes = std::stod(
				line.substr(stringPos + 1, std::string::npos));
		//sync_cout<< fen << " " << gameRes <<sync_endl;
		src.pos.setupFromFen(fen);
		src.limits.infinite = true;
		src.limits.depth = 0;
		startThinkResult res = src.startThinking();
		double sig = sigmoid(res.score, scaling);
		double error = gameRes - sig;
		//sync_cout <<res.score<<" "<<sig<<" "<<gameRes<<" "<<error<<sync_endl;
		totalError += std::pow(error, 2.0);
		//std::list<Move> PV = res.PV;
		//sync_cout << "bestmove " << displayUci( PV.front() )<<sync_endl;
		count++;
	}
	totalError /= count;
	input.close();
	return totalError;
}

void tune()
{
	simdScore* values[4] ={&knightOnOutpost,&knightOnOutpostSupported,&knightOnHole,&KnightAttackingWeakPawn};

	sync_cout<<"calc initial Error"<<sync_endl;
	double initialError = calcError();
	double bestError = initialError;
	sync_cout<<"initial Error "<<std::setprecision(10)<<initialError<<sync_endl;

	bool improved = true;
	while(improved)
	{
		improved = false;
		for( int valNumber = 0;valNumber<4;valNumber++)
		{
			for( int valSub = 0;valSub<2;valSub++)
			{
				sync_cout <<"param["<<valNumber+1<<","<<valSub <<"]"<<sync_endl;

				int originalVal = (*values[valNumber])[valSub];

				// try +1
				int newVal = originalVal+1;
				(*values[valNumber])[valSub] = newVal;
				double newError = calcError();
				sync_cout <<"+1: "<<newVal<< std::setprecision(10)<<" "<<newError <<sync_endl;

				if(newError < bestError)
				{
					improved = true;
					bestError = newError;
					sync_cout <<"ok +1: "<<newVal<<sync_endl;
				}
				else
				{
					//try -1
					newVal = originalVal-1;
					(*values[valNumber])[valSub] = newVal;
					double newError = calcError();
					sync_cout <<"-1: "<<newVal<< std::setprecision(10)<<" "<<newError <<sync_endl;

					if(newError < bestError)
					{
						improved = true;
						bestError = newError;
						sync_cout <<"ok -1: "<<newVal<<sync_endl;
					}
					else
					{
						//revert to original value
						newVal = originalVal;
						(*values[valNumber])[valSub] = newVal;
						sync_cout <<"back to original: "<<newVal<<sync_endl;
					}
				}


			}
		}
	}



}


