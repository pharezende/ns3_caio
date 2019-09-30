/*
 * slice-Orchestrator.cc
 *
 *  Created on: 11 de jul de 2017
 *      Author: pedrohenrique
 */

#include <ns3/slice-orchestrator.h>


using namespace std;

namespace ns3 {

//Fazer um mapeamento RNTI -> Slice.

SliceOrchestrator::SliceOrchestrator() {
	// TODO Auto-generated constructor stub
}

void SliceOrchestrator::CreateSlice(int enodebID, int percentage,
		vector<int> imsis, int typeOfService, int priority, int sliceID) {
	Ptr<Slice> slice = Create<Slice> ();
	slice->SetUEs(imsis);
	slice->SetTypeOfService(typeOfService);
	Attributes attr = m_enodebID_Attributes[enodebID];
	int totalRBsFrame = (attr.nbRBsFrame * percentage) / 100;
	slice->SetNbRBs(totalRBsFrame);
	slice->SetPriority(priority);
	attr.slices.push_back(slice);
	attr.sliceCounter = attr.sliceCounter + 1;
	slice->SetId(sliceID);
	m_enodebID_Attributes[enodebID].slices.push_back(slice);
	m_enodebID_Attributes[enodebID] = attr;
}

void SliceOrchestrator::DestroySlice() {
}

void SliceOrchestrator::UpdateSlice(/*Ptr<Slice> p_slice*/) {
}


void SliceOrchestrator::ReadMatrixFile(int **& matrix, int row, int col, std::string fileName) // read matrix from file
{
	int i, j;
	ifstream file;
	file.open(fileName);  // open file for reading
	for (i = 0; i < row; i++)   // row loop
	{
		for (j = 0; j < col; j++)  // column loop
		{
			file >> matrix[i][j]; // read data into matrix
		}
	}
	file.close(); // close the file
}

int** SliceOrchestrator::CreateSlicingMatrix(int dlBandwidth, std::string fileName){ //Quebrar galho.

	int** matrix;
	if(dlBandwidth == 25){ // 5 Mhz -> 2 RB per RBG -> 13 RBGs (Lines)
		matrix = new int*[13]; //Row
		int i;
		for(i=0; i<13; i++){
			matrix[i] = new int[10];
		}
		ReadMatrixFile(matrix, 13, 10, fileName);

	}
	/*else if(dlBandwidth == 50){ // 10 Mhz -> 3 RB per RBG -> 17 RBGs (Lines)

	}
	else if(dlBandwidth == 75){ // 15 Mhz -> 4 RB per RBG -> 19 RBGs (Lines)

	}*/
	else if(dlBandwidth == 100){ // 20 Mhz -> 4 RB per RBG -> 25 RBGs (Lines)
		matrix = new int*[25]; //Row
		int i;
		for(i=0; i<25; i++){
			matrix[i] = new int[10];
		}
		ReadMatrixFile(matrix, 25, 10, fileName);

	}
	return matrix;
}

int** SliceOrchestrator::CreateUplinkSlicingMatrix(int ulBandwidth, std::string fileName){ //Quebrar galho.

	int** matrix;
	if(ulBandwidth == 25){ // 5 Mhz -> 25 RBs
		int lines = 26;
		matrix = new int*[lines]; //Row
		int i;
		for(i=0; i<lines; i++){
			matrix[i] = new int[10];
		}
		ReadMatrixFile(matrix, lines, 10, fileName);
	}
	/*else if(dlBandwidth == 50){ // 10 Mhz

	}
	else if(dlBandwidth == 75){ // 15 Mhz

	}*/
	else if(ulBandwidth == 100){ // 20 Mhz
		int lines = 101;
		matrix = new int*[lines]; //Row
		int i;
		for(i=0; i<lines; i++){
			matrix[i] = new int[10];
		}
		ReadMatrixFile(matrix, lines, 10, fileName);

	}
	return matrix;
}

void SliceOrchestrator::SendSlicingMatrix(int enodebID, std::string channel, int rbgsNumberPerSubframe, std::string matrixFile ){
	//Deixar isso aqui estático
	Attributes attributes = m_enodebID_Attributes[enodebID];
	SlicesOptimizer* sliceOptimizer = attributes.sliceOptimizer;
	if (channel.compare("downlink") == 0) {
		attributes.m_schedulingMatrix = CreateSlicingMatrix(attributes.bandwidth, matrixFile);
		sliceOptimizer->ReceiveMatrixScheduling(attributes.m_schedulingMatrix, rbgsNumberPerSubframe, 10);
	}
	if (channel.compare("uplink") == 0) {
		attributes.m_schedulingMatrix = CreateUplinkSlicingMatrix(attributes.bandwidth, matrixFile);
		sliceOptimizer->ReceiveMatrixScheduling(attributes.m_schedulingMatrix, rbgsNumberPerSubframe, 10);
	}
}


void SliceOrchestrator::SendSlices(int enodebID) {
	SlicesOptimizer* sliceOptimizer = m_enodebID_Attributes[enodebID].sliceOptimizer;
	vector<Ptr<Slice>> slices = m_enodebID_Attributes[enodebID].slices;
	sliceOptimizer->ReceiveSlices(slices);
}



void SliceOrchestrator::ReceiveSliceOptimizer(int enodebID, int totalRBsFrame, int bandwidth,
		SlicesOptimizer* sliceOptimizer) {
	Attributes attr = m_enodebID_Attributes[enodebID];
	if(attr.sliceCounter == -1){
		attr.nbRBsFrame = totalRBsFrame;
		attr.sliceCounter++;
		attr.bandwidth = bandwidth;
		attr.sliceOptimizer = sliceOptimizer;
		m_enodebID_Attributes[enodebID] = attr;
	}
	else{
		attr.sliceOptimizer = sliceOptimizer;
	}
}

SliceOrchestrator::~SliceOrchestrator() {
	// TODO Auto-generated destructor stub
}

}

