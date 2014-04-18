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

	sync_cout<<"ALL"<<sync_endl;
	for(int i=0;i<16*ONE_PLY;i++){
		sync_cout<<i<<" ";
		std::cout<<testedAllPruning[i]<<" ";
		std::cout<<correctAllPruning[i]<<" ";
		std::cout<<(signed long long)(correctAllPruning[i])/float(testedAllPruning[i])*100.0<<sync_endl;
	}
	sync_cout<<"CUT"<<sync_endl;
	for(int i=0;i<16*ONE_PLY;i++){
			sync_cout<<i<<" ";
			std::cout<<testedAllPruning[i]<<" ";
			std::cout<<correctAllPruning[i]<<" ";
			std::cout<<(signed long long)(correctAllPruning[i])/float(testedAllPruning[i])*100.0<<sync_endl;
		}


	sync_cout<<"Cut node ordering performance: "<<(signed long long)(cutNodeOrderingSum)/float(cutNodeOrderingCounter)<<" "<<cutNodeOrderingCounter<<sync_endl;
	sync_cout<<"worst move ordering: "<<worstCutNodeOrdering<<std::endl<<sync_endl;
	sync_cout<<"all node ordering performance: "<<(signed long long)(allNodeOrderingSum)/float(allNodeOrderingCounter)<<std::endl<<sync_endl;
	sync_cout<<"pv node ordering performance: "<<(signed long long)(pvNodeOrderingSum)/float(pvNodeOrderingCounter)<<std::endl<<sync_endl;

	for(int i=0;i<200;i++){
		sync_cout<< i<<" "<<cutNodeOrderingArray[i]<<sync_endl;
	}


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

	for(int i=0;i<4*ONE_PLY;i++){
		testedAll=0;

		testedAllPruning[i]=0;
		correctAllPruning[i]=0;

		testedCut=0;
		testedCutPruning[i]=0;
		correctCutPruning[i]=0;
	}

	cutNodeOrderingSum=0;
	cutNodeOrderingCounter=0;
	worstCutNodeOrdering=0;

	allNodeOrderingSum=0;
	allNodeOrderingCounter=0;

	pvNodeOrderingSum=0;
	pvNodeOrderingCounter=0;

	for(int i=0;i<200;i++){
		cutNodeOrderingArray[i]=0;
	}
}


