/*
 * slice-orchestrator.h
 *
 *  Created on: 11 de jul de 2017
 *      Author: pedrohenrique
 */

#ifndef NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_SLICING_SLICE_ORCHESTRATOR_H_
#define NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_SLICING_SLICE_ORCHESTRATOR_H_

#include <ns3/slice.h> //Importa todas as bibliotecas declaradas em slice.h
#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>
#include <ns3/slices-optimizer.h>
#include <fstream>

//NS3 - New Model Example (https://www.nsnam.org/docs/manual/html/new-models.html)
using namespace std;
namespace ns3 {

class SliceOrchestrator: public SimpleRefCount<SliceOrchestrator> {

public:

	void UpdateSlice();
	void CreateSlice(int enodebID, int percentage, vector<int> imsis, int typeOfService, int priority, int sliceID);
	void DestroySlice();
	void SendSlices(int enodebID);
	void ReceiveSliceOptimizer(int enodebID, int totalRBsFrame, int dlBandwidth, SlicesOptimizer* sliceOptimizer);
	void SendSlicingMatrix(int enodebID, std::string channel, int rbgsNumberPerSubframe, std::string matrixFile);
	void ReadMatrixFile(int **& matrix, int row, int col, std::string fileName);
	int** CreateSlicingMatrix(int dlBandwidth, std::string fileName);
	int** CreateUplinkSlicingMatrix(int ulBandwidth, std::string fileName);
	//void ReceiveStats();
	SliceOrchestrator();
	virtual ~SliceOrchestrator();

private:
	struct Attributes{
		vector<Ptr<Slice> > slices;
		SlicesOptimizer* sliceOptimizer;
		int sliceCounter = -1; //
		int nbRBsFrame = 0;
		int **m_schedulingMatrix;
		int bandwidth;
	};
	map<int, Attributes> m_enodebID_Attributes;
};

}

/*
 I use:

 m for members
 c for constants/readonlys
 p for pointer (and pp for pointer to pointer)
 v for volatile
 s for static
 i for indexes and iterators
 e for events

 */

#endif /* NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_SLICING_SLICE_ORCHESTRATOR_H_ */
