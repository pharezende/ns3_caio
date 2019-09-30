/*
 * sdn-controller.cc
 *
 *  Created on: 12 de jul de 2017
 *      Author: pedrohenrique
 */

#include <ns3/sdn-controller.h>

using namespace std;
namespace ns3 {

SDNController::SDNController() {
	// TODO Auto-generated constructor stub

}

SDNController::~SDNController() {
	// TODO Auto-generated destructor stub
}

SDNController::SDNController(Ptr<SliceOrchestrator> sliceOrchestrator) {
	// TODO Auto-generated constructor stub
	m_sliceOrchestrator = sliceOrchestrator;
}

void SDNController::ReceiveServiceDescriptors(int enodebID, int typeOfService,
		vector<int> rntis, int sliceID) {
	int porcentage;
	//int priority;
	if(typeOfService == 1){
		porcentage = 20; //Ou colocar na granularidade de grupo?
		//priority = 3; //Video
	}
	else if(typeOfService == 2){
		porcentage = 30;
		//priority = 2; //VoiP
	}
	else if(typeOfService == 3){
		porcentage = 50;
		//priority = 1; //FTP
	}
	else{
		porcentage = 100;
	}
	//Como pegar ENodeB? Entrar no Descritor de Serviço?
	m_sliceOrchestrator->CreateSlice(enodebID, porcentage, rntis, typeOfService, typeOfService, sliceID);
}

void SDNController::VerifyPolicies(int userID) {
}

Ptr<SliceOrchestrator>
SDNController::GetSliceOrchestrator() {
	return m_sliceOrchestrator;
}

}
