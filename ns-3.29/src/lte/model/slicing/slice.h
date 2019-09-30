/*
 * slice.h
 *
 *  Created on: 11 de jul de 2017
 *      Author: pedrohenrique
 */

#ifndef NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_SLICING_SLICE_H_
#define NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_SLICING_SLICE_H_
#include <vector>
#include <ns3/simple-ref-count.h>

using namespace std;
namespace ns3 {

class Slice : public SimpleRefCount<Slice> { //Libera apenas o ponteiro. (https://www.nsnam.org/docs/manual/html/object-model.html#object-model)
public:
	Slice();
	virtual ~Slice();
	int GetId();
	void SetId(int id);
	int GetNbRBs();
	void SetNbRBs(int nbRBs);
	int GetTypeOfService();
	void SetTypeOfService(int typeOfService);
	vector<int> GetUEs();
	void SetUEs(vector<int> rntis);
	int GetPriority();
	void SetPriority(int priority);

private:
	int m_id;
	int m_nbRBs;
	int m_typeOfService;
	int m_priority;
	vector<int> m_ues; //UEs que estão nesse Slice.

};

}

#endif /* NS_ALLINONE_3_26_NS_3_26_SRC_LTE_MODEL_SLICING_SLICE_H_ */
