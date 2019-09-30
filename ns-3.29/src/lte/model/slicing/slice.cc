/*
 * slice.cc
 *
 *  Created on: 11 de jul de 2017
 *      Author: pedrohenrique
 */

#include <ns3/slice.h>

using namespace std;
namespace ns3 {

Slice::Slice() {
	// TODO Auto-generated constructor stub

}

Slice::~Slice() {
	// TODO Auto-generated destructor stub
}

int Slice::GetId() {
	return m_id;
}

void Slice::SetId(int id) {
	m_id = id;
}

int Slice::GetNbRBs() {
	return m_nbRBs;
}

void Slice::SetNbRBs(int nbRBs) {
	m_nbRBs = nbRBs;
}

int Slice::GetTypeOfService() {
	return m_typeOfService;
}

void Slice::SetTypeOfService(int typeOfService) {
	m_typeOfService = typeOfService;
}

vector<int> Slice::GetUEs() {
	return m_ues;
}

void Slice::SetUEs(vector<int> rntis) {
	m_ues = rntis;
}

int Slice::GetPriority(){
	return m_priority;
}

void Slice::SetPriority(int priority) {
	m_priority = priority;
}

}
