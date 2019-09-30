/*
 * slices-optimizer.cpp
 *
 *  Created on: 9 de ago de 2017
 *      Author: pedro
 */

#include <ns3/slices-optimizer.h>

using namespace std;
namespace ns3 {

SlicesOptimizer::SlicesOptimizer() {
	// TODO Auto-generated constructor stub

}

SlicesOptimizer::~SlicesOptimizer() {
	// TODO Auto-generated destructor stub
}

int Type0AllocationRbg[4] = {
  10,       // RGB size 1
  26,       // RGB size 2
  63,       // RGB size 3
  110       // RGB size 4
};  // see table 7.1.6.1-1 of 36.213

int
SlicesOptimizer::GetRbgSize (int dlbandwidth)
{
  for (int i = 0; i < 4; i++)
    {
      if (dlbandwidth < Type0AllocationRbg[i])
        {
          return (i + 1);
        }
    }

  return (-1);
}

void SlicesOptimizer::ReceiveSlices(vector<Ptr<Slice> > slices) {
	for (uint16_t i = 0; i < slices.size(); i++) {
		Ptr<Slice> slice = slices.at(i);
		vector<int> uesIMSIs = slice->GetUEs();
		for (uint16_t j = 0; j < uesIMSIs.size(); j++) {
			int imsi = uesIMSIs.at(j);
			m_UE_To_Slice[imsi] = slice;
		}
		m_Slices.push_back(slice);
		m_SliceID_to_SliceData[slice->GetId()] = 0;
		//m_SliceID_To_RBGs_Available[slice->GetId()] = slice->GetNbRBs() / m_RBGSize;
	}
}


vector<int> SlicesOptimizer::SelectSlice(int rbg){

	struct Stats{
		int counter; //Quantos RLCs querem transmitir. Dependendo pode tirar isso depois.
		int rlcsTransmissionQueueSize; //O tamanho total das filas dos RLCs
		int priority; //Prioridade do Slice
	};
	vector<int> totalUEs;
	map<int, Stats> SliceID_To_Counter_Map;
	uint32_t i;
	for(i=0; i< m_Slices.size(); i++){ //Inicializa SliceID_To_Counter_Map
		Ptr<Slice> slice = m_Slices[i];
		Stats stats;
		int id = slice->GetId();
		stats.counter = 0;
		stats.rlcsTransmissionQueueSize = 0;
		stats.priority = slice->GetPriority();
		SliceID_To_Counter_Map[id] = stats;
		vector<int> ues = slice->GetUEs();
		totalUEs.insert(totalUEs.end(), ues.begin(), ues.end());
	}

	if(m_Dynamic_Slicing_Flag == false || m_RlcBuffer.empty() == true){
		//Não usa Slices Dinâmico
		int sliceID = GetSliceIDAccordingToRBGFlag(rbg);
		uint32_t j = 0;
		for(j = 0; j < m_Slices.size(); j++){
			int id = m_Slices[j]->GetId();
			if(id == sliceID){
				return m_Slices[j]->GetUEs();
			}
		}
	}

	//Verifica se o slice definido para o rbg tem dados para transmitir.
	int selectedSliceID = 1;
	int sliceID = GetSliceIDAccordingToRBGFlag(rbg);
	if(m_RBG_Flag == false && m_alreadyChosenDynamic > 0){
		uint32_t j;
		for(j = 0; j < m_Slices.size(); j++){
			int id = m_Slices[j]->GetId();
			if(id == m_alreadyChosenDynamic)
				return m_Slices[j]->GetUEs();
		}
	}

	if(m_RBG_Flag == true){
		if(sliceID == m_lastSliceFromArray){
			uint32_t j;
			for(j = 0; j < m_Slices.size(); j++){
				int id = m_Slices[j]->GetId();
				if(id == m_lastSliceSelected)
					return m_Slices[j]->GetUEs();
			}
		}
	}

	if(m_SliceID_to_SliceData[sliceID] > 0){ //Escalonar o slice definido pela matrix de alocação de recursos se ele tiver tráfego.
		selectedSliceID = sliceID;
	}
	else{ //Pegar um outro que tenha algo para transmitir.
		map<int, int>::iterator itSliceData;
		int maxTraffic = 0;
		int maxPriority = 0;
		int sliceID = 1;
		int amountOfData = 0;
		for(itSliceData = m_SliceID_to_SliceData.begin(); itSliceData != m_SliceID_to_SliceData.end();
				itSliceData++){

			sliceID = itSliceData->first;
			amountOfData = itSliceData->second;
			if(m_SliceID_to_SliceData[sliceID] == 0){
				continue;
			}

			if(SliceID_To_Counter_Map[sliceID].priority > maxPriority){ //Sempre pegar o slice de maior prioridade
				maxTraffic = amountOfData;
				maxPriority = SliceID_To_Counter_Map[sliceID].priority;
				selectedSliceID = sliceID;
			}
			else if(m_SliceID_to_SliceData[sliceID] > maxTraffic && SliceID_To_Counter_Map[sliceID].priority == maxPriority){ //Em caso de empate de prioridade, pegue o Slice com mais tráfego para transmitir.
				maxTraffic = m_SliceID_to_SliceData[sliceID];
				selectedSliceID = sliceID;
			}
		}
	}

	if(m_RBG_Flag == true){
		if(sliceID != m_lastSliceFromArray){
			m_lastSliceFromArray = sliceID;
			m_lastSliceSelected = selectedSliceID;
		}
	}


	//Selecionar UEs que podem ser escalonados nesse subframe.
	uint32_t j;
	for(j = 0; j < m_Slices.size(); j++){
		int id = m_Slices[j]->GetId();
		if(id == selectedSliceID)
			break;
	}


	if(m_RBG_Flag == false && m_alreadyChosenDynamic == -1){
		m_alreadyChosenDynamic = selectedSliceID;
	}

	return m_Slices[j]->GetUEs();
}

void SlicesOptimizer::PopulateCQIStatus(std::map<uint16_t, bool> imsiToStatus){
	m_imsiToStatus = imsiToStatus;
	int k = 0;
	k++;
}

vector<int> SlicesOptimizer::SelectUplinkSlice(int rbg){

	struct Stats{
		int counter; //Quantos RLCs querem transmitir. Dependendo pode tirar isso depois.
		int rlcsTransmissionQueueSize; //O tamanho total das filas dos RLCs
		int priority; //Prioridade do Slice
	};
	vector<int> totalUEs;
	map<int, Stats> SliceID_To_Counter_Map;
	uint32_t i;
	for(i=0; i< m_Slices.size(); i++){ //Inicializa SliceID_To_Counter_Map
		Ptr<Slice> slice = m_Slices[i];
		Stats stats;
		int id = slice->GetId();
		stats.counter = 0;
		stats.rlcsTransmissionQueueSize = 0;
		stats.priority = slice->GetPriority();
		SliceID_To_Counter_Map[id] = stats;
		vector<int> ues = slice->GetUEs();
		totalUEs.insert(totalUEs.end(), ues.begin(), ues.end());
	}

	if(m_Dynamic_Slicing_Flag == false){
		//Não usa Slices Dinâmico
		int sliceID = GetSliceIDAccordingToRBGFlag(rbg);
		uint32_t j = 0;
		for(j = 0; j < m_Slices.size(); j++){
			int id = m_Slices[j]->GetId();
			if(id == sliceID){
				return m_Slices[j]->GetUEs();
			}
		}
	}

	//Verifica se o slice definido para o rbg tem dados para transmitir.
	int selectedSliceID = 11;
	int sliceID = GetSliceIDAccordingToRBGFlag(rbg);
	if(m_RBG_Flag == false && m_alreadyChosenDynamicUplink > 0){
		uint32_t j;
		for(j = 0; j < m_Slices.size(); j++){
			int id = m_Slices[j]->GetId();
			if(id == m_alreadyChosenDynamicUplink)
				return m_Slices[j]->GetUEs();
		}
	}

	/*//Adicionei agora trecho abaixo:
	bool flagCQI = false;
	std::map<int, bool> availableCQISlicesMAP;
	uint32_t j;
	for (j = 0; j < m_Slices.size(); j++) {
		Ptr<Slice> slice = m_Slices[j];
		if(sliceID == slice->GetId()){
			vector<int> imsis = slice->GetUEs();
			uint32_t i;
			for (i = 0; i < imsis.size(); i++) {
				int imsi = imsis[i];
				if (m_imsiToStatus[imsi] == true) {
					flagCQI = true;
					break;
				}
			}
		}
	}*/
	//
	if(m_SliceID_to_SliceData[sliceID] > 0 /*&& flagCQI = true*/){ //Escalonar o slice definido pela matrix de alocação de recursos se ele tiver tráfego.
		selectedSliceID = sliceID;
	}
	else{ //Pegar um outro que tenha algo para transmitir.

		//Adicionei abaixo
		/*std::map<int, bool> availableCQISlicesMAP;
		if(m_RBG_Flag == true){
			uint32_t j;
			for(j = 0; j < m_Slices.size(); j++){
				Ptr<Slice> slice = m_Slices[j];
				vector<int> imsis = slice->GetUEs();
				uint32_t i;
				availableCQISlicesMAP[slice->GetId()] = false;
				for(i=0; i< imsis.size(); i++){
					int imsi = imsis[i];
					if(m_imsiToStatus[imsi] == true){
						availableCQISlicesMAP[slice->GetId()] = true;
						break;
					}
				}
			}
		}*/
		//--------------------------------------------

		map<int, int>::iterator itSliceData;
		int maxTraffic = 0;
		int maxPriority = 0;
		for(itSliceData = m_SliceID_to_SliceData.begin(); itSliceData != m_SliceID_to_SliceData.end();
				itSliceData++){

			int sliceID = itSliceData->first;
			int amountOfData = itSliceData->second;
			if(m_SliceID_to_SliceData[sliceID] == 0){
				continue;
			}

			/*if(m_RBG_Flag == true){
				if(availableCQISlicesMAP[sliceID] == false){ //Nenhum CQI dos UEs eh maior que 0
					continue;
				}
			}*/

			if(SliceID_To_Counter_Map[sliceID].priority > maxPriority){ //Sempre pegar o slice de maior prioridade
				maxTraffic = amountOfData;
				maxPriority = SliceID_To_Counter_Map[sliceID].priority;
				selectedSliceID = sliceID;
			}
			else if(m_SliceID_to_SliceData[sliceID] > maxTraffic && SliceID_To_Counter_Map[sliceID].priority == maxPriority){ //Em caso de empate de prioridade, pegue o Slice com mais tráfego para transmitir.
				maxTraffic = m_SliceID_to_SliceData[sliceID];
				selectedSliceID = sliceID;
			}
		}
	}
	//Selecionar UEs que podem ser escalonados nesse subframe.
	uint32_t j;
	for(j = 0; j < m_Slices.size(); j++){
		int id = m_Slices[j]->GetId();
		if(id == selectedSliceID)
			break;
	}

	if(m_RBG_Flag == false && m_alreadyChosenDynamicUplink == -1){
		m_alreadyChosenDynamicUplink = selectedSliceID;
	}

	//printf("%d\n", selectedSliceID);
	return m_Slices[j]->GetUEs();
}

int SlicesOptimizer::CalculateOffset(int beginIndex){
	vector<int> vector = m_SubFrame_To_Slices[m_CurrentSubFrame - 1];
	uint32_t i = beginIndex;
	int sliceID = vector[beginIndex]; //Mesmo no dinamico, quando pode mudar o sliceID, aproveito o valor para calcular offset
	while(vector[i] == sliceID && (i < vector.size())){
		i++;
	}
	return i - beginIndex;
}

int SlicesOptimizer::GetSliceIDAccordingToRBGFlag(int rbg){
	int sliceID = 0;
	if(m_RBG_Flag == false){
		sliceID = m_Matrix_Scheduling[0][(m_CurrentSubFrame - 1)];
	}
	else{
		sliceID = m_Matrix_Scheduling[rbg][(m_CurrentSubFrame - 1)];
	}
	return sliceID;
}

int SlicesOptimizer::GetNbRBGs() {
	return m_NbRBGs;
}

void SlicesOptimizer::SetNbRBGs(int nbRBGs) {
	m_NbRBGs = nbRBGs;
}

void SlicesOptimizer::InsertRowRNTIToIMSIMap(int rnti, int imsi) {
	m_RNTI_To_IMSI[rnti] =  imsi;
}

void SlicesOptimizer::InsertRowIMSIToRNTIMap(int imsi, int rnti) {
	m_IMSI_To_RNTI[imsi] =  rnti;
}

map<int, int> SlicesOptimizer::GetIMSIToRNTIMap() {
	return m_IMSI_To_RNTI;
}

void SlicesOptimizer::SetRBGSize(int rbgSize) {
	m_RBGSize = rbgSize;
}

void SlicesOptimizer::SetCurrentSubFrame(int subFrame) {
	m_CurrentSubFrame = subFrame;
}

void SlicesOptimizer::SetCurrentFrame(int frame) {
	m_CurrentFrame = frame;
}

//Percorro todos os RLCs para ver se alguém tem dado para ser enviado desse slice. Caso não tenha, selecione um outro slice.

void SlicesOptimizer::ReceiveMatrixScheduling(int **matriz_scheduling,
		int rbg, int subframes) {
	m_Matrix_Scheduling = matriz_scheduling;
	int i, j;
	for (i = 0; i < subframes; i++) {
		set<int> uniqueSlicesInSubframeSet;
		vector<int> vec;
		for (j = 0; j < rbg; j++) {
			uniqueSlicesInSubframeSet.insert(matriz_scheduling[j][i]);
			vec.push_back(matriz_scheduling[j][i]);
		}
		m_SubFrame_To_Slices[i] = vec;
		m_SubframeToUniqueSlices[i] =  uniqueSlicesInSubframeSet;
	}
}

void SlicesOptimizer::UpdateRlcBufferRetransmission(int rnti, int value){
	m_RlcBufferRetransmission[rnti] = value;
}

void SlicesOptimizer::SetRowRlcBufferTransmission(int rnti, int txQueueSize) {
	m_RlcBuffer[rnti] = txQueueSize;
}

int SlicesOptimizer::UpdateRowRlcBuffer(int rnti, int size, bool flag) { //flag = true -> Soma || flag = false -> Diferença
	if(flag == true)
		m_RlcBuffer[rnti] += size;
	else{
		int reduz;
		if(m_RlcBuffer[rnti] > size){
			reduz = size;
		}
		else{
			reduz = m_RlcBuffer[rnti];
		}
		m_RlcBuffer[rnti] -= reduz;
		return reduz;
		/*if(m_RlcBuffer[rnti] < 0){
			m_RlcBuffer[rnti] = 0;
		}*/
	}
	return 0;
}

map<int, int> SlicesOptimizer::GetRNTIToIMSIMap() {
	return m_RNTI_To_IMSI;
}

void SlicesOptimizer::SetDynamicSlicingFlag(bool value) {
	m_Dynamic_Slicing_Flag = value;
}

void SlicesOptimizer::SetRBGFlag(bool value){
	m_RBG_Flag = value;
}

void SlicesOptimizer::SetShareOptionFlag(bool value){
	m_Share_Option_Flag = value;
}

int SlicesOptimizer::GetAmountOfBufferData(int rnti){
	return m_RlcBuffer[rnti];
}

void SlicesOptimizer::IncreaseAmountOfSliceData(int rnti, int size){
	Ptr<Slice> slice = m_UE_To_Slice[m_RNTI_To_IMSI[rnti]];
	int sliceID = slice->GetId();
	m_SliceID_to_SliceData[sliceID] += size;
}

void SlicesOptimizer::ComputeSliceDataSize(){
	ClearSlicesData();
	Ptr<Slice> slice;
	size_t i;
	for(i=0; i<m_Slices.size(); i++){
		slice = m_Slices.at(i);
		vector<int> ues = slice->GetUEs();
		size_t j;
		for(j=0; j< ues.size(); j++){
			int data = m_RlcBuffer[m_IMSI_To_RNTI[ues[j]]];
			m_SliceID_to_SliceData[slice->GetId()] += data;
		}
	}
}


void SlicesOptimizer::DecreaseAmountOfSliceData(int rnti, int size){
	Ptr<Slice> slice = m_UE_To_Slice[m_RNTI_To_IMSI[rnti]];
	int sliceID = slice->GetId();
	m_SliceID_to_SliceData[sliceID] -= size;
	if(m_SliceID_to_SliceData[sliceID] < 0){
		m_SliceID_to_SliceData[sliceID] = 0;
	}
}

int  SlicesOptimizer::GetAmountOfSliceData(int rnti){
	Ptr<Slice> slice = m_UE_To_Slice[m_RNTI_To_IMSI[rnti]];
	int sliceID = slice->GetId();
	return m_SliceID_to_SliceData[sliceID];
}

bool SlicesOptimizer::GetDynamicSlicingFlag(){
	return m_Dynamic_Slicing_Flag;
}

void SlicesOptimizer::ClearSlicesData() {
	map<int, int>::iterator it;
	for (it = m_SliceID_to_SliceData.begin();
			it != m_SliceID_to_SliceData.end(); it++) {
		m_SliceID_to_SliceData[it->first] = 0;
	}
}

vector<int> SlicesOptimizer::GetSlicebyID(int id) {
	vector<Ptr<Slice>>::iterator it;
	for (it = m_Slices.begin();
			it != m_Slices.end(); it++) {
		Ptr<Slice> slice = *it;
		if(slice->GetId() == id)
			return slice->GetUEs();
	}
	vector<int> aux;
	return aux;
}

bool SlicesOptimizer::GetRBGFlag(){
	return m_RBG_Flag;
}

bool SlicesOptimizer::GetShareOptionFlag(){
	return m_Share_Option_Flag;
}

std::set<int> SlicesOptimizer::GetSlicesPresentInSubframe(){
	return m_SubframeToUniqueSlices[m_CurrentSubFrame - 1];
}

void SlicesOptimizer::SetFirstSlice(int sliceID){
	m_FirstSlice = sliceID;
}

void SlicesOptimizer::SetNumberOfUEs(size_t nbUEs){
	m_NumberOfUEs = nbUEs;
}

size_t SlicesOptimizer::GetNumberOfUEs(){
	return m_NumberOfUEs;
}

void SlicesOptimizer::SetUsedRBsDownlinkTrace(std::string traceAddress) {
	m_downlinkTrace = traceAddress;
}

void SlicesOptimizer::SetUsedRBsUplinkTrace(std::string traceAddress) {
	m_uplinkTrace = traceAddress;
}

std::string SlicesOptimizer::GetUsedRBsDownlinkTrace() {
	return m_downlinkTrace;
}

std::string SlicesOptimizer::GetUsedRBsUplinkTrace() {
	return m_uplinkTrace;
}

}

