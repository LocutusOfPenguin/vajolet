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
	sync_cout<<"nodetype stats"<<std::endl<<sync_endl;

	sync_cout<<"tested nodetype PV: "<<testedNodeTypePv<<sync_endl;
	sync_cout<<"tested nodetype Cut: "<<testedNodeTypeCut<<sync_endl;
	sync_cout<<"tested nodetype All: "<<testedNodeTypeAll<<std::endl<<sync_endl;

	sync_cout<<"result nodetype PV: "<<resultNodeTypePv<<sync_endl;
	sync_cout<<"result nodetype Cut: "<<resultNodeTypeCut<<sync_endl;
	sync_cout<<"result nodetype All: "<<resultNodeTypeAll<<std::endl<<sync_endl;

	sync_cout<<"PV type %error : "<<(signed long long)(testedNodeTypePv-resultNodeTypePv)/float(resultNodeTypePv)*100.0<<sync_endl;
	sync_cout<<"Cut type %error : "<<(signed long long)(testedNodeTypeCut-resultNodeTypeCut)/float(resultNodeTypeCut)*100.0<<sync_endl;
	sync_cout<<"All type %error : "<<(signed long long)(testedNodeTypeAll-resultNodeTypeAll)/float(resultNodeTypeAll)*100.0<<std::endl<<sync_endl;


	sync_cout<<"tested Pv razoring: "<<testedPvPruning<<sync_endl;
	sync_cout<<"passed Pv razoring: "<<passedPvPruning<<sync_endl;
	sync_cout<<"correct Pv razoring: "<<correctPvPruning<<sync_endl;
	sync_cout<<"% correct Pv pruning: "<<(signed long long)(correctPvPruning)/float(passedPvPruning)*100.0<<std::endl<<sync_endl;

	sync_cout<<"tested All razoring: "<<testedAllPruning<<sync_endl;
	sync_cout<<"passed All razoring: "<<passedAllPruning<<sync_endl;
	sync_cout<<"correct All razoring: "<<correctAllPruning<<sync_endl;
	sync_cout<<"% correct All pruning: "<<(signed long long)(correctAllPruning)/float(passedAllPruning)*100.0<<std::endl<<sync_endl;

	sync_cout<<"tested Cut razoring: "<<testedCutPruning<<sync_endl;
	sync_cout<<"passed Cut razoring: "<<passedCutPruning<<sync_endl;
	sync_cout<<"correct Cut razoring: "<<correctCutPruning<<sync_endl;
	sync_cout<<"% correct Cut pruning: "<<(signed long long)(correctCutPruning)/float(passedCutPruning)*100.0<<std::endl<<sync_endl;

	sync_cout<<"Cut razoring node count : "<<countNodeTestedPruning<<sync_endl;
	sync_cout<<"NO Cut razoring node count: "<<countNodeTestedNoPruning<<sync_endl;
	sync_cout<<"% saved Cut pruning: "<<(signed long long)(countNodeTestedNoPruning-countNodeTestedPruning)/float(countNodeTestedNoPruning)*100.0<<std::endl<<sync_endl;
	sync_cout<<"avg nodes per pruning: "<<(signed long long)(countNodeTestedPruning)/float(testedCutPruning+testedAllPruning+testedPvPruning)<<std::endl<<sync_endl;

}

void Statistics::gatherNodeTypeStat(search::nodeType expectedNodeType,search::nodeType resultNodeType){
	switch(expectedNodeType){
	case search::nodeType::ROOT_NODE:
		testedNodeTypePv++;
		break;
	case search::nodeType::PV_NODE:
		testedNodeTypePv++;
		break;
	case search::nodeType::CUT_NODE:
		testedNodeTypeCut++;
		break;
	case search::nodeType::ALL_NODE:
		testedNodeTypeAll++;
		break;

	}


	switch(resultNodeType){
	case search::nodeType::ROOT_NODE:
		resultNodeTypePv++;
		break;
	case search::nodeType::PV_NODE:
		resultNodeTypePv++;
		break;
	case search::nodeType::CUT_NODE:
		resultNodeTypeCut++;
		break;
	case search::nodeType::ALL_NODE:
		resultNodeTypeAll++;
		break;

	}
}

void Statistics::initNodeTypeStat(){
	testedNodeTypeCut=0;
	testedNodeTypeAll=0;
	testedNodeTypePv=0;
	resultNodeTypeCut=0;
	resultNodeTypeAll=0;
	resultNodeTypePv=0;

	testedAllPruning=0;
	passedAllPruning=0;
	correctAllPruning=0;

	testedCutPruning=0;
	passedCutPruning=0;
	correctCutPruning=0;

	testedPvPruning=0;
	passedPvPruning=0;
	correctPvPruning=0;

	countNodeTestedPruning=0;
	countNodeTestedNoPruning=0;
}


