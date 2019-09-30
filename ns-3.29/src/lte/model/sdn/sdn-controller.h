/*
 * sdn-controller.h
 *
 *  Created on: 12 de jul de 2017
 *      Author: pedrohenrique
 */

#ifndef NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_SDN_SDN_CONTROLLER_H_
#define NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_SDN_SDN_CONTROLLER_H_

#include <ns3/ptr.h>
#include <ns3/slice-orchestrator.h> //Já importa todos as bibliotecas necessárias.

using namespace std;
namespace ns3 {

class SDNController: public SimpleRefCount<SDNController> {
public:
	SDNController();
	SDNController(Ptr<SliceOrchestrator> sliceOrchestrator);
	virtual ~SDNController();
	void ReceiveServiceDescriptors(int enodebID, int typeOfService,
			vector<int> rntis, int sliceID); //Descritor de Serviços
	void VerifyPolicies(int userID); //Depois
	Ptr<SliceOrchestrator> GetSliceOrchestrator();


private:
	Ptr<SliceOrchestrator> m_sliceOrchestrator;

};

}

#endif /* NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_SDN_SDN_CONTROLLER_H_ */
