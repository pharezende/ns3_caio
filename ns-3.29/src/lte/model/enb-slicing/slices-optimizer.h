/*
 * slices-optimizer.h
 *
 *  Created on: 9 de ago de 2017
 *      Author: pedro
 */

#ifndef NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_ENB_SLICING_SLICES_OPTIMIZER_H_
#define NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_ENB_SLICING_SLICES_OPTIMIZER_H_

#include <ns3/slice.h>
#include <ns3/ptr.h>
#include <map>
#include <set>

using namespace std;
namespace ns3 {

class SlicesOptimizer{
public:

	SlicesOptimizer();
	virtual ~SlicesOptimizer();
	int GetRbgSize (int dlbandwidth);
	void ReceiveSlices(vector<Ptr<Slice> > slices);
	vector<int> SelectSlice(int rbg);
	void PopulateCQIStatus(std::map<uint16_t, bool> imsiToStatus);
	vector<int> SelectUplinkSlice(int rbg);
	int CalculateOffset(int beginIndex);
	int GetSliceIDAccordingToRBGFlag(int rbg);
	int GetNbRBGs();
	void SetNbRBGs(int nbRBGs);
	void InsertRowRNTIToIMSIMap(int rnti, int imsi);
	void InsertRowIMSIToRNTIMap(int imsi, int rnti);
	void SetRBGSize(int rbgSize);
	void SetCurrentSubFrame(int subFrame);
	void SetCurrentFrame(int frame);
	void SetDynamicSlicingFlag(bool value);
	void SetRBGFlag(bool value);
	void SetShareOptionFlag(bool value);
	void ReceiveMatrixScheduling(int **matrix_scheduling, int rbg, int subframes);
	void UpdateRlcBufferRetransmission(int rnti, int value);
	void SetRowRlcBufferTransmission(int rnti, int txQueueSize);
	int UpdateRowRlcBuffer(int rnti, int size, bool flag);
	map<int, int> GetRNTIToIMSIMap();
	int GetAmountOfBufferData(int rnti);
	void IncreaseAmountOfSliceData(int rnti, int size);
	void ComputeSliceDataSize();
	void DecreaseAmountOfSliceData(int rnti, int size);
	int GetAmountOfSliceData(int rnti);
	bool GetDynamicSlicingFlag();
	void ClearSlicesData();
	vector<int> GetSlicebyID(int id);
	bool GetRBGFlag();
	bool GetShareOptionFlag();
	map<int, int> GetIMSIToRNTIMap();
	std::set<int> GetSlicesPresentInSubframe();
	void SetFirstSlice(int sliceID);
	void SetNumberOfUEs(size_t nbUEs);
	size_t GetNumberOfUEs();
	void SetUsedRBsDownlinkTrace(std::string traceAddress);
	void SetUsedRBsUplinkTrace(std::string traceAddress);
	std::string GetUsedRBsDownlinkTrace();
	std::string GetUsedRBsUplinkTrace();
	int m_CurrentSubFrame;
	int m_CurrentFrame;
	map<int, int> m_SliceID_to_SliceData;
	int m_alreadyChosenDynamic = -1; //So para o Dinamico sem RBG
	int m_alreadyChosenDynamicUplink = -1;//So para o Dinamico sem RBG
	int m_lastSliceFromArray = -1; //So para o Dinamico com RBG
	int m_lastSliceSelected = -1; //So para o Dinamico com RBG

private:

	map<int, vector<int>> m_SubFrame_To_Slices;
	//map<int, int> m_SliceID_to_SliceData;
	map<int, int> m_IMSI_To_RNTI;
	map<int, int> m_RNTI_To_IMSI;
	map<int, Ptr<Slice>> m_UE_To_Slice;
	map<int, int> m_SliceID_To_RBGs_Available;
	map <int, int> m_RlcBufferRetransmission;
	map <int, int> m_RlcBuffer; //Ignorando os outros. Ver: LteEnbMac::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
	vector<Ptr<Slice>> m_Slices;
	map<int, set<int>> m_SubframeToUniqueSlices;
	int** m_Matrix_Scheduling;
	int m_NbRBGs;
	int m_RBGSize;
	int m_FirstSlice;
	int m_NumberOfUEs;
	bool m_Dynamic_Slicing_Flag;
	bool m_RBG_Flag;
	bool m_Share_Option_Flag;
	std::string m_downlinkTrace;
	std::string m_uplinkTrace;
	std::map<uint16_t, bool> m_imsiToStatus;
};

}
#endif /* NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_ENB_SLICING_SLICES_OPTIMIZER_H_ */
