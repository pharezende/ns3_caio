/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */

#include "ns3/flow-monitor-helper.h"
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/sdn-controller.h"
#include "ns3/random-variable-stream.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "../src/core/model/nstime.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;
//using namespace std;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */

//Precisa desabilitar o binding do Python ---> ./waf configure --disable-python
NS_LOG_COMPONENT_DEFINE("EpcFirstExample");
std::string defaultPath = "/home/pedro/ns-allinone-3.29/";

struct attributes{
	uint64_t rxBytes;
	int64_t  delaySUM;
	int64_t  jitterSUM;
	float   packetsDropped;
	uint32_t rxPackets;
	uint32_t txPackets;
};

struct qosmetricsaggregated{
	uint64_t throughput;
	int64_t  delay;
	int64_t  jitter;
	float   packetsDropped;
	size_t   counter;
	size_t   counter_2;
	uint32_t txPackets;//Usado apenas no método GatherQoSStatistics
};


std::map<int, attributes> flowMonitorPastFields;
std::map<int, qosmetricsaggregated> sliceQoSMetricsPerSecond;
double timeInstant = 0;
std::string monitoringTraceAddress;
std::string monitoringThroughputTraceAddress;
std::string monitoringDelayTraceAddress;
std::string monitoringJitterTraceAddress;
std::string monitoringLostPacketsTraceAddress;

std::map<int, std::ofstream*> sliceIDToOutfileMonitoringThroughputTrace; //Um por Slice;
std::map<int, std::ofstream*> sliceIDToOutfileMonitoringDelayTrace;
std::map<int, std::ofstream*> sliceIDToOutfileMonitoringJitterTrace;
std::map<int, std::ofstream*> sliceIDToOutfileMonitoringLostPacketsTrace;

std::ofstream outfileMonitoringTrace;
std::vector<int> slicesIDs = {1, 2, 3, 4, 5, 11, 12, 13, 14, 15, 16};
std::map<int, double> slicesIDToTimeDuration;

bool Trace_MonitoringPerSecond = false;
bool Trace_SplitMonitoringPerSecond = false;
bool Trace_AggregateStats = false;
double httpSimTime = 80.0;


void InstantiateTraces(std::string seed_NbOFUEs, std::string tipoAlocacao, std::string granularidade){

    for(std::vector<int>::iterator sliceIDsIT = slicesIDs.begin();
    			sliceIDsIT != slicesIDs.end(); ++sliceIDsIT){

    	int sliceID = *sliceIDsIT;
    	std::stringstream seed_NbOFUEs_copy;
    	seed_NbOFUEs_copy << seed_NbOFUEs;
    	seed_NbOFUEs_copy << "_" << sliceID;
    	std::string monitoringThroughputTraceAddress = defaultPath+"ns-3.29/monitoring_per_second/throughput/throughput_trace_"
    			+tipoAlocacao+"_"+granularidade+"_"+seed_NbOFUEs_copy.str();

    	std::string monitoringDelayTraceAddress = defaultPath+"ns-3.29/monitoring_per_second/delay/delay_trace_"
    			+tipoAlocacao+"_"+granularidade+"_"+seed_NbOFUEs_copy.str();

    	std::string monitoringJitterTraceAddress = defaultPath+"ns-3.29/monitoring_per_second/jitter/jitter_trace_"
    			+tipoAlocacao+"_"+granularidade+"_"+seed_NbOFUEs_copy.str();

    	std::string monitoringLostPacketsTraceAddress = defaultPath+"ns-3.29/monitoring_per_second/lost_packets/lost_packets_trace_"
    			+tipoAlocacao+"_"+granularidade+"_"+seed_NbOFUEs_copy.str();

    	std::ofstream* outfileMonitoringThroughputTrace = new ofstream(); //Um por Slice;
    	std::ofstream* outfileMonitoringDelayTrace = new ofstream();
    	std::ofstream* outfileMonitoringJitterTrace = new ofstream();
    	std::ofstream* outfileMonitoringLostPacketsTrace = new ofstream();

    	outfileMonitoringThroughputTrace->open(monitoringThroughputTraceAddress, ios::out);
    	outfileMonitoringDelayTrace->open(monitoringDelayTraceAddress, ios::out);
    	outfileMonitoringJitterTrace->open(monitoringJitterTraceAddress, ios::out);
    	outfileMonitoringLostPacketsTrace->open(monitoringLostPacketsTraceAddress, ios::out);

    	sliceIDToOutfileMonitoringThroughputTrace.insert(std::make_pair(sliceID,outfileMonitoringThroughputTrace)); //Ou abaixo
    	sliceIDToOutfileMonitoringDelayTrace[sliceID] = outfileMonitoringDelayTrace;
    	sliceIDToOutfileMonitoringJitterTrace[sliceID] = outfileMonitoringJitterTrace;
    	sliceIDToOutfileMonitoringLostPacketsTrace[sliceID] = outfileMonitoringLostPacketsTrace;

    }

}

void GatherQoSStatistics(FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon, std::string finalDest , double simTime)
{
	flowMon->CheckForLostPackets();
	std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
	Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
	std::map<int, qosmetricsaggregated> sliceIDToStats;
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
	{
		int flowID = stats->first;

		Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (flowID);
		uint16_t sliceID = fiveTuple.sliceNumber;
		struct qosmetricsaggregated qosStats;
		if(sliceIDToStats.count(sliceID) == 0){
			qosStats.throughput = 0;
			qosStats.delay = 0;
			qosStats.jitter = 0;
			qosStats.packetsDropped = 0;
			qosStats.counter = 0;
			qosStats.txPackets = 0;
			sliceIDToStats[sliceID] = qosStats;
		}

		qosStats = sliceIDToStats[sliceID];
		qosStats.delay += stats->second.delaySUM.GetNanoSeconds() / stats->second.rxPackets;
		qosStats.jitter += stats->second.jitterSUM.GetNanoSeconds() / stats->second.rxPackets;
		Time difference = Time::FromInteger ((slicesIDToTimeDuration[sliceID]), Time::S) - stats->second.timeFirstTxPacket;
		qosStats.throughput += ((8.0*stats->second.rxBytes)/ 1000) / (1.0*difference.GetMilliSeconds() / 1000); //Kbits
		qosStats.packetsDropped += stats->second.lostPackets;
		qosStats.txPackets += stats->second.txPackets;
		qosStats.counter++;
		sliceIDToStats[sliceID] = qosStats;
	}

	std::ofstream outfileMonitoringDownlinkThroughputTrace(defaultPath+"ns-3.29/monitoring_aggregated/throughput/downlink_trace_"+finalDest,
			ios::out);
	std::ofstream outfileMonitoringDownlinkDelayTrace(defaultPath+"ns-3.29/monitoring_aggregated/delay/downlink_trace_"+finalDest,
			ios::out);
	std::ofstream outfileMonitoringDownlinkJitterTrace(defaultPath+"ns-3.29/monitoring_aggregated/jitter/downlink_trace_"+finalDest,
			ios::out);
	std::ofstream outfileMonitoringDownlinkPacketLossTrace(defaultPath+"ns-3.29/monitoring_aggregated/lost_packets/downlink_trace_"+finalDest,
			ios::out);

	std::ofstream outfileMonitoringUplinkThroughputTrace(defaultPath+"ns-3.29/monitoring_aggregated/throughput/uplink_trace_"+finalDest,
			ios::out);
	std::ofstream outfileMonitoringUplinkDelayTrace(defaultPath+"ns-3.29/monitoring_aggregated/delay/uplink_trace_"+finalDest,
			ios::out);
	std::ofstream outfileMonitoringUplinkJitterTrace(defaultPath+"ns-3.29/monitoring_aggregated/jitter/uplink_trace_"+finalDest,
			ios::out);
	std::ofstream outfileMonitoringUplinkPacketLossTrace(defaultPath+"ns-3.29/monitoring_aggregated/lost_packets/uplink_trace_"+finalDest,
			ios::out);

	for(std::map<int, qosmetricsaggregated>::const_iterator sliceIDsIT = sliceIDToStats.begin();
				sliceIDsIT != sliceIDToStats.end(); ++sliceIDsIT){

		int sliceID = sliceIDsIT->first;
		qosmetricsaggregated metrics = sliceIDsIT->second;


		if(metrics.throughput == 0 && metrics.counter >= 0){ // O slice 3 nao vai entrar quando o tempo de execucao eh menor que 40.
			continue;
		}

		if(sliceID == 3){
			int k=0;
			k++;
		}

		float droppedPacketsPercentageAggregated = (metrics.packetsDropped * 100)/metrics.txPackets;
		if(sliceID <= 5){ //Downlink

			outfileMonitoringDownlinkThroughputTrace << metrics.throughput / metrics.counter << "\n";
	    	outfileMonitoringDownlinkDelayTrace << metrics.delay / (metrics.counter * 1000000) << "\n";
	    	outfileMonitoringDownlinkJitterTrace << metrics.jitter / (metrics.counter * 1000000) << "\n";
	    	outfileMonitoringDownlinkPacketLossTrace << droppedPacketsPercentageAggregated/metrics.counter << "\n";

		}

		if(sliceID >= 11){ //Uplink

			outfileMonitoringUplinkThroughputTrace << metrics.throughput / metrics.counter << "\n";
			outfileMonitoringUplinkDelayTrace << metrics.delay / (metrics.counter * 1000000) << "\n";
			outfileMonitoringUplinkJitterTrace << metrics.jitter / (metrics.counter * 1000000) << "\n";
			outfileMonitoringUplinkPacketLossTrace  << droppedPacketsPercentageAggregated/metrics.counter << "\n";
		}
	}

	outfileMonitoringDownlinkThroughputTrace.close();
	outfileMonitoringDownlinkDelayTrace.close();
	outfileMonitoringDownlinkJitterTrace.close();
	outfileMonitoringDownlinkPacketLossTrace.close();

	outfileMonitoringUplinkThroughputTrace.close();
	outfileMonitoringUplinkDelayTrace.close();
	outfileMonitoringUplinkJitterTrace.close();
	outfileMonitoringUplinkPacketLossTrace.close();

}

void QoSMonitorEachSecond (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon, double simTime)
{
	double offset = 1;
	outfileMonitoringTrace << "Instant" << "\t";
	outfileMonitoringTrace << timeInstant << std::endl;
	flowMon->CheckForLostPackets();
	std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
	Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
	{
		int flowID = stats->first;
		Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
		uint16_t sliceID = fiveTuple.sliceNumber;

		if(sliceID == 3){
			int k=0;
			k++;
		}

		if((stats->second.rxBytes) > 0){
			attributes pastValues;
			if(flowMonitorPastFields.count(flowID) == 0){
				pastValues.rxBytes = 0;
				pastValues.delaySUM = 0;
				pastValues.jitterSUM = 0;
				pastValues.packetsDropped = 0;
				pastValues.rxPackets = 0;
				pastValues.txPackets = 0;
				flowMonitorPastFields[flowID] = pastValues;
			}

			pastValues = flowMonitorPastFields[flowID];
			uint64_t throughput;
			uint64_t delay;
			uint64_t jitter;
			uint32_t txDifference;
			float	packetDropped;
			txDifference = stats->second.txPackets - pastValues.txPackets;
			throughput = ((stats->second.rxBytes - pastValues.rxBytes) * 8.0) / (offset * 1000); //kbps
			if((stats->second.rxPackets - pastValues.rxPackets) == 0){
				packetDropped = 0;
				delay = 0;
				jitter = 0;
			}
			else{
				packetDropped = (stats->second.lostPackets - pastValues.packetsDropped); //Passar para porcentagem
				delay =  (stats->second.delaySUM.GetNanoSeconds() - pastValues.delaySUM) /
						(stats->second.rxPackets - pastValues.rxPackets); //ns
				jitter =  (stats->second.jitterSUM.GetNanoSeconds() - pastValues.jitterSUM) /
						(stats->second.rxPackets - pastValues.rxPackets); //ns
			}

			pastValues.rxBytes = stats->second.rxBytes;
			pastValues.delaySUM = stats->second.delaySUM.GetNanoSeconds();
			pastValues.jitterSUM = stats->second.jitterSUM.GetNanoSeconds();
			pastValues.packetsDropped += packetDropped;
			pastValues.rxPackets = stats->second.rxPackets;
			pastValues.txPackets = stats->second.txPackets;

			flowMonitorPastFields[flowID] = pastValues;

			qosmetricsaggregated metrics;
			if(sliceQoSMetricsPerSecond.count(sliceID) == 0){
				metrics.throughput = 0;
				metrics.delay = 0;
				metrics.jitter = 0;
				metrics.packetsDropped = 0;
				metrics.counter = 0;
				metrics.counter_2 = 0;
				metrics.txPackets = 0;
				sliceQoSMetricsPerSecond[sliceID] = metrics;
			}
			metrics = sliceQoSMetricsPerSecond[sliceID];
			metrics.throughput+= throughput;
			metrics.delay +=  delay;
			metrics.jitter += jitter;
			metrics.packetsDropped += packetDropped;
			metrics.txPackets += txDifference;

			if(delay > 0.1)
				metrics.counter++;
			metrics.counter_2++;

			sliceQoSMetricsPerSecond[sliceID] = metrics;
		}
	}

	for(std::vector<int>::iterator sliceIDsIT = slicesIDs.begin();
				sliceIDsIT != slicesIDs.end(); ++sliceIDsIT){
		int sliceID = *sliceIDsIT;
		qosmetricsaggregated metrics;
		outfileMonitoringTrace << sliceID << "\t";
		std::ofstream* outfileMonitoringThroughputTrace = sliceIDToOutfileMonitoringThroughputTrace[sliceID];
		std::ofstream* outfileMonitoringDelayTrace = sliceIDToOutfileMonitoringDelayTrace[sliceID];
		std::ofstream* outfileMonitoringJitterTrace = sliceIDToOutfileMonitoringJitterTrace[sliceID];
		std::ofstream* outfileMonitoringLostPacketsTrace = sliceIDToOutfileMonitoringLostPacketsTrace[sliceID];

		if(sliceQoSMetricsPerSecond.count(sliceID) == 0){
			if(Trace_MonitoringPerSecond == true){
				outfileMonitoringTrace << 0 << "\t";
				outfileMonitoringTrace << 0 << "\t";
				outfileMonitoringTrace << 0 << "\t";
				outfileMonitoringTrace << 0 << "\n";
			}
			if(Trace_SplitMonitoringPerSecond == true){
				*outfileMonitoringThroughputTrace << 0 << "\t";
				*outfileMonitoringDelayTrace << 0 << "\t";
				*outfileMonitoringJitterTrace << 0 << "\t";
				*outfileMonitoringLostPacketsTrace << 0 << "\t";
			}
		}
		else{
			metrics = sliceQoSMetricsPerSecond.at(sliceID);
			if(metrics.throughput == 0){
				if(Trace_MonitoringPerSecond == true){
					outfileMonitoringTrace << 0 << "\t";
					outfileMonitoringTrace << 0 << "\t";
					outfileMonitoringTrace << 0 << "\t";
					outfileMonitoringTrace << 0 << "\n";
				}
				if(Trace_SplitMonitoringPerSecond == true){
					*outfileMonitoringThroughputTrace << 0 << "\t";
					*outfileMonitoringDelayTrace << 0 << "\t";
					*outfileMonitoringJitterTrace << 0 << "\t";
					*outfileMonitoringLostPacketsTrace << 0 << "\t";
				}
			}

			else{
				float droppedPacketsPercentageAggregated = (metrics.packetsDropped * 100)/metrics.txPackets;
				if(isnan(droppedPacketsPercentageAggregated)){
					droppedPacketsPercentageAggregated = 0;
				}
				if(Trace_MonitoringPerSecond == true){
					outfileMonitoringTrace << metrics.throughput/metrics.counter_2 << "\t";
					outfileMonitoringTrace << metrics.delay/(metrics.counter * 1000000) << "\t"; //ms
					outfileMonitoringTrace << metrics.jitter/(metrics.counter * 1000000) << "\t"; //ms
					outfileMonitoringTrace << (droppedPacketsPercentageAggregated/metrics.counter_2) << "\n";
				}
				if(Trace_SplitMonitoringPerSecond == true){
					*outfileMonitoringThroughputTrace << metrics.throughput/metrics.counter_2 << "\t";
					*outfileMonitoringDelayTrace << metrics.delay/(metrics.counter * 1000000) << "\t";
					*outfileMonitoringJitterTrace << metrics.jitter/(metrics.counter * 1000000) << "\t";
					*outfileMonitoringLostPacketsTrace << (droppedPacketsPercentageAggregated/metrics.counter_2) << "\t";
				}
			}
		}
		metrics.throughput = 0;
		metrics.delay = 0;
		metrics.jitter = 0;
		metrics.packetsDropped = 0;
		metrics.txPackets = 0;
		metrics.counter = 0;
		metrics.counter_2 = 0;
		sliceQoSMetricsPerSecond[sliceID] = metrics;

		if (Trace_MonitoringPerSecond == true) {
			outfileMonitoringTrace << std::flush;
		}
		if (Trace_SplitMonitoringPerSecond == true) {
			*outfileMonitoringThroughputTrace << std::flush;
			*outfileMonitoringDelayTrace << std::flush;
			*outfileMonitoringJitterTrace << std::flush;
			*outfileMonitoringLostPacketsTrace << std::flush;
		}
	}
   timeInstant+=offset;

   Simulator::Schedule(Seconds(offset), &QoSMonitorEachSecond, fmhelper, flowMon, simTime);

}


void updateMatrix_1(int enbID, Ptr<SliceOrchestrator> sliceOrchestrator, Ptr<SliceOrchestrator> uplinkSliceOrchestrator,
		int nbRBGsSubframeDownlink, int nbRBGsSubframeUplink){

	sliceOrchestrator->SendSlicingMatrix(enbID, "downlink", nbRBGsSubframeDownlink,
			defaultPath + "ns-3.29/matrix_slicing_definition/downlinkMatrix20MHz_HTTP");

	uplinkSliceOrchestrator->SendSlicingMatrix(enbID, "uplink", nbRBGsSubframeUplink,
			defaultPath + "ns-3.29/matrix_slicing_definition/uplinkMatrix20MHz_HTTP");
}

void updateMatrix_2(int enbID, Ptr<SliceOrchestrator> sliceOrchestrator, Ptr<SliceOrchestrator> uplinkSliceOrchestrator,
		int nbRBGsSubframeDownlink, int nbRBGsSubframeUplink){

	sliceOrchestrator->SendSlicingMatrix(enbID, "downlink", nbRBGsSubframeDownlink,
			defaultPath + "ns-3.29/matrix_slicing_definition/downlinkMatrix20MHz");

	uplinkSliceOrchestrator->SendSlicingMatrix(enbID, "uplink", nbRBGsSubframeUplink,
			defaultPath + "ns-3.29/matrix_slicing_definition/uplinkMatrix20MHz");
}


int main(int argc, char *argv[]) {
	LogComponentEnable ("EpcFirstExample", LOG_INFO);
	Config::SetDefault("ns3::TcpL4Protocol::SocketType",
			StringValue("ns3::TcpVegas"));
	/*##########################################
	4 Modos de Execucao:
	Estatico + Subframe: estatico subframe
	Dinamico + Subframe: dinamico subframe
	Estatico + RBG: estatico rbg
	Dinamico + RBG: dinamico rbg
	##########################################*/

	if (argc < 12) { // We expect 6 arguments
		std::cerr << "São necessários 11 argumentos!!!!" << std::endl;
		return 1;
	}
	bool DYNAMICSLICING = false;
	bool RBGGRANULARITY = false;
	bool SHARE_SLICE_BETWEEN_UEs = false; //Durante o escalonamento, dois ou mais UEs podem compartilhar os recursos
	bool BASH_ARGS = false;
	bool Trace_USEDRBs = false;


	//Default: 5 MHz -> 25 PRBs
	Ptr<FlowMonitor> flowMonitor;
	Ptr<FlowMonitor> flowMonitor2;
	FlowMonitorHelper flowHelper;
	FlowMonitorHelper flowHelper2;
	std::string tipoAlocacao;
	std::string granularidade;
	std::string share_option;
	std::string monitoring;
	std::string split_monitoring;
	std::string aggregate_stats;

	double simTime;
	//7 Slices
	uint16_t nbOfUes; //Pedestres - VoIP -- Dois
	uint16_t nbOfUes2; //Pedestres - FTP -- Dois
	uint16_t nbOfUes3; //Ciclistas - Video -- Uplink
	uint16_t nbOfUes4; //Veiculo - HTTP -- Dois
	uint16_t nbOfUes5; //Veiculo - VoIP -- Dois
	uint16_t nbOfUes6; //Veiculo - Video -- Downlink
	uint16_t nbOfUes7; //Semaforo - Video -- Uplink
	uint32_t seed;
	//#ECLIPSE
	//Slices 1, 2, 3 e 5 possuem os dois canais
	if(BASH_ARGS == false){
		tipoAlocacao = argv[1];
		granularidade = argv[2];
		share_option = argv[3];
		monitoring = argv[4];
		split_monitoring = argv[5];
		aggregate_stats = argv[6];
		simTime = atof(argv[7]);
		nbOfUes = atoi(argv[8]);
		nbOfUes2 = atoi(argv[9]);
		nbOfUes3 = atoi(argv[10]);
		nbOfUes4 = atoi(argv[11]);
		nbOfUes5 = atoi(argv[12]);
		nbOfUes6 = atoi(argv[13]);
		nbOfUes7 = atoi(argv[14]);
		seed = atoi(argv[15]);
	}
	//####

	// Command line arguments
	CommandLine cmd;
	if(BASH_ARGS == true){
		cmd.AddValue("tipoAlocacao", "Tipo de Alocacao", tipoAlocacao);
		cmd.AddValue("granularidade", "Granularidade", granularidade);
		cmd.AddValue("share_option", "Share Option", share_option);
		cmd.AddValue("monitoring", "Um unico trace", monitoring);
		cmd.AddValue("split_monitoring", "Traces distintos para cada metrica", split_monitoring);
		cmd.AddValue("aggregate_stats", "Agregado estatisticas", aggregate_stats);
		cmd.AddValue("simTime", "Tempo de Simulacao", simTime);
		cmd.AddValue("nbOfUes", "UE1s", nbOfUes);
		cmd.AddValue("nbOfUes2", "UE2s", nbOfUes2);
		cmd.AddValue("nbOfUes3", "UE3s", nbOfUes3);
		cmd.AddValue("nbOfUes4", "UE4s", nbOfUes4);
		cmd.AddValue("nbOfUes5", "UE5s", nbOfUes5);
		cmd.AddValue("nbOfUes6", "UE6s", nbOfUes6);
		cmd.AddValue("nbOfUes7", "UE7s", nbOfUes7);
		cmd.AddValue("seed", "seed", seed);
		cmd.Parse(argc, argv);
	}


	//Duracao Slices, para dividir no final.
	slicesIDToTimeDuration[1] = simTime;
	slicesIDToTimeDuration[2] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[3] = httpSimTime;
	slicesIDToTimeDuration[4] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[5] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[11] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[12] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[13] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[14] = httpSimTime;
	slicesIDToTimeDuration[15] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[16] = slicesIDToTimeDuration[1];

	Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320));

	std::string videoByc=defaultPath+"ns-3.29/movies/park_cam_medium.dat";
	std::string videoVeic=defaultPath+"ns-3.29/movies/formula_1_high.dat";
	std::string videoSemaforo=defaultPath+"ns-3.29/movies/office_cam_high.dat";
	uint16_t nbOfENodeBs = 1;
	RngSeedManager::SetSeed(seed);
	RngSeedManager::SetRun(7);
	if (tipoAlocacao.compare("dinamico") == 0) {
		DYNAMICSLICING = true;
	}
	if (granularidade.compare("rbg") == 0){
		RBGGRANULARITY = true;
	}
	if (share_option.compare("share_option") == 0){ //Apenas para o Downlink! Uplink(RR) ja contempla isso.
		SHARE_SLICE_BETWEEN_UEs = true;
	}

	if(monitoring.compare("monitoring") == 0){
		Trace_MonitoringPerSecond = true;
	}

	if(split_monitoring.compare("split_monitoring") == 0){
		Trace_SplitMonitoringPerSecond = true;
	}

	if(aggregate_stats.compare("aggregate_stats") == 0){
		Trace_AggregateStats = true;
	}



	std::string downlinkTraceAddress;
	std::string uplinkTraceAddress;
	if(Trace_USEDRBs == true){
		stringstream seedAux;
		seedAux << seed;
		downlinkTraceAddress = defaultPath+"ns-3.29/used_rbs_traces/downlink_"+tipoAlocacao+"_"+granularidade+"_"+seedAux.str();
		uplinkTraceAddress = defaultPath+"ns-3.29/used_rbs_traces/uplink_"+tipoAlocacao+"_"+granularidade+"_"+seedAux.str();
		std::ofstream outfile (downlinkTraceAddress);
		outfile.close();
		std::ofstream outfile2 (uplinkTraceAddress);
		outfile2.close();
	}


	uint16_t sum = nbOfUes + nbOfUes2 + nbOfUes3 + nbOfUes4 + nbOfUes5 + nbOfUes6 + nbOfUes7;
	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults();

	/*int nbRBsDownlink = 25;
	int nbRBGsSubframeDownlink = 13;*/
	int nbRBsDownlink = 100;
	int nbRBGsSubframeDownlink = 25;
	int nbRBsUplink = 100;
	int nbRBGsSubframeUplink = 100;
	/*int nbRBsUplink = 100;
	int nbRBGsSubframeUplink = 100;*/

	//Um novo ID é criado para cada dispositivo do NodeContainer, partindo do 0. O mapeamento eh o mesmo que o definido no trace SUMO.
	//Prestar muita atenção!!!!!

	NodeContainer ueNodes;
	NodeContainer ueNodes2;
	NodeContainer ueNodes3;
	NodeContainer ueNodes4;
	NodeContainer ueNodes5;
	NodeContainer ueNodes6;
	NodeContainer ueNodes7;

	ueNodes.Create(nbOfUes);
	ueNodes2.Create(nbOfUes2);
	ueNodes3.Create(nbOfUes3);
	ueNodes4.Create(nbOfUes4);
	ueNodes5.Create(nbOfUes5);
	ueNodes6.Create(nbOfUes6);
	ueNodes7.Create(nbOfUes7);

	/****
	 * DEFINIR MOBILIDADE
	 */




	Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
	Ptr<PointToPointEpcHelper> epcHelper =
			CreateObject<PointToPointEpcHelper>();
	lteHelper->SetEpcHelper(epcHelper);
	lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");

	//#######################################DOWNLINK#################################################

	//######Fading Trace Model ---> FALSO!
	lteHelper->SetAttribute("FadingModel",
			StringValue("ns3::TraceFadingLossModel"));

	std::string traceName = "fading_trace_ETU_3kmph.fad"; //Trace falso, só para evitar problemas no LTE-Helper.
	std::string baseAddress = "src/lte/model/fading-traces/" + traceName; //Terminal
	std::string alternativeAddress = defaultPath + "ns-3.29/" + baseAddress; //IDE

	std::ifstream ifTraceFile;
	ifTraceFile.open(alternativeAddress, std::ifstream::in);
	if (ifTraceFile.good()) {
		// script launched by test.py
		lteHelper->SetFadingModelAttribute("TraceFilename",
				StringValue(alternativeAddress));
	} else {
		// script launched as an example
		lteHelper->SetFadingModelAttribute("TraceFilename",
				StringValue(baseAddress));
	}

	lteHelper->SetFadingModelAttribute("TraceLength",
			TimeValue(Seconds(simTime)));
	lteHelper->SetFadingModelAttribute("SamplesNum", UintegerValue(120000));
	lteHelper->SetFadingModelAttribute("WindowSize", TimeValue(Seconds(0.5)));
	lteHelper->SetFadingModelAttribute("RbNum", UintegerValue(nbRBsDownlink));
	//#####################

	Ptr<Node> pgw = epcHelper->GetPgwNode();

	// Create a single RemoteHost
	NodeContainer remoteHostContainer;
	remoteHostContainer.Create(1);
	Ptr<Node> remoteHost = remoteHostContainer.Get(0);
	InternetStackHelper internet;
	internet.Install(remoteHostContainer);

	// Create the Internet
	PointToPointHelper p2ph;
	p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
	p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
	p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.001))); //Não é somado no atraso que aparece no trace
	NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
	Ipv4AddressHelper ipv4h;
	ipv4h.SetBase("1.0.0.0", "255.0.0.0"); //Rede PGW - R.Host
	Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
	// interface 0 is localhost, 1 is the p2p device
	//Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress(1);

	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
			ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
	remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"),
			Ipv4Mask("255.0.0.0"), 1);
	//7.0.0.0 -> Rede ENodeB (RAN)

	int radius = 100; //OK
	// Position of eNBs
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<
			ListPositionAllocator>();
	positionAlloc->Add(Vector(radius, radius, 0.0));
	//positionAlloc->Add (Vector (enbDist, 0.0, 0.0));
	NodeContainer enbNodes;
	NodeContainer epcRemoteHost;
	enbNodes.Create(nbOfENodeBs);
	MobilityHelper enbMobility;
	enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	enbMobility.SetPositionAllocator(positionAlloc);
	enbMobility.Install(enbNodes);
	epcRemoteHost.Add(pgw);
	epcRemoteHost.Add(remoteHost);
	enbMobility.Install(epcRemoteHost); //Só para definir a posição estática.
	enbMobility.AssignTraceID(enbNodes, -1); //-1 é só para comparar no trace-fading-loss.

	std::string mobilitySUMOFile;
	std::string sumoUEsSeed;
	std::string path;
	stringstream somaAux;
	stringstream seedAux;

	//Remover a partir daqui!
 	/*MobilityHelper ue1mobility;
 	ue1mobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator", "X",
 			DoubleValue(0.0), "Y", DoubleValue(0.0), "rho",
 			DoubleValue(radius));

 	ue1mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel", "Speed",
 			StringValue("ns3::ConstantRandomVariable[Constant=0.83]"), //(m/s) -> 3KM/h -> 0,83m/s
			"Bounds", RectangleValue(Rectangle(0, radius*2, 0, radius*2)));
 	ue1mobility.Install(ueNodes);
 	ue1mobility.Install(ueNodes2);
 	ue1mobility.Install(ueNodes3);
 	ue1mobility.Install(ueNodes4);
 	ue1mobility.Install(ueNodes5);
 	ue1mobility.Install(ueNodes6);

 	ue1mobility.AssignTraceID(ueNodes, 1); //Colocar 1 para cada Slice.
 	ue1mobility.AssignTraceID(ueNodes2, 1);
 	ue1mobility.AssignTraceID(ueNodes3, 2);
 	ue1mobility.AssignTraceID(ueNodes4, 3);
 	ue1mobility.AssignTraceID(ueNodes5, 3);
 	ue1mobility.AssignTraceID(ueNodes6, 3);*/

 	//##############################

	uint16_t sum_semUEs7 = nbOfUes + nbOfUes2 + nbOfUes3 + nbOfUes4 + nbOfUes5 + nbOfUes6;
	std::string sumString = std::to_string(sum_semUEs7);

	std::string baseX = "ns-3.29/mobility/" + sumString + "/";
	//Slices 1 e 11
	somaAux << (nbOfUes + nbOfUes2);
	seedAux << 1; //Usar apenas 1 seed na mobilidade
	sumoUEsSeed = somaAux.str() + "_" + seedAux.str();
	path = defaultPath + baseX + "mobility_peds/slice_1/ns2_ped_" + sumoUEsSeed + "_slice_1.tcl";
	Ns2MobilityHelper pedMobility_1 = Ns2MobilityHelper (path);
	pedMobility_1.DefineTraceID(1);
	pedMobility_1.Install ();

	//Slices 2 e 12
	somaAux.str(std::string());
	somaAux.clear();
	seedAux.str(std::string());
	seedAux.clear();
	somaAux << (nbOfUes + nbOfUes2);
	seedAux << 1;
	sumoUEsSeed = somaAux.str() + "_" + seedAux.str();
	path = defaultPath + baseX + "mobility_peds/slice_2/ns2_ped_" + sumoUEsSeed + "_slice_2.tcl";
	Ns2MobilityHelper pedMobility_2 = Ns2MobilityHelper (path);
	pedMobility_2.DefineTraceID(1);
	pedMobility_2.Install ();

	//Slice 13
	somaAux.str(std::string());
	somaAux.clear();
	seedAux.str(std::string());
	seedAux.clear();
	somaAux << (nbOfUes3);
	seedAux << 1;
	sumoUEsSeed = somaAux.str() + "_" + seedAux.str();
	path = defaultPath + baseX + "mobility_cycs/slice_3/ns2_cyc_" + sumoUEsSeed + "_slice_3.tcl";
	Ns2MobilityHelper cicMobility_1 = Ns2MobilityHelper (path);
	cicMobility_1.DefineTraceID(2);
	cicMobility_1.Install ();

	//Slices 3 e 14 ---> Mudar para veiculo
	somaAux.str(std::string());
	somaAux.clear();
	seedAux.str(std::string());
	seedAux.clear();
	somaAux << (nbOfUes4+nbOfUes5 + nbOfUes6);
	seedAux << 1;
	sumoUEsSeed = somaAux.str() + "_" + seedAux.str();
	path = defaultPath + baseX + "mobility_vehs/slice_4/ns2_veh_" + sumoUEsSeed + "_slice_4.tcl";
	Ns2MobilityHelper vehMobility_1 = Ns2MobilityHelper (path);
	vehMobility_1.DefineTraceID(3);
	vehMobility_1.Install ();

	//Slices 4 e 15
	somaAux.str(std::string());
	somaAux.clear();
	seedAux.str(std::string());
	seedAux.clear();
	somaAux << (nbOfUes4+nbOfUes5 + nbOfUes6);
	seedAux << 1;
	sumoUEsSeed = somaAux.str() + "_" + seedAux.str();
	path = defaultPath + baseX + "mobility_vehs/slice_5/ns2_veh_" + sumoUEsSeed + "_slice_5.tcl";
	Ns2MobilityHelper vehMobility_2 = Ns2MobilityHelper (path);
	vehMobility_2.DefineTraceID(3);
	vehMobility_2.Install ();

	//Slice 5
	somaAux.str(std::string());
	somaAux.clear();
	seedAux.str(std::string());
	seedAux.clear();
	somaAux << (nbOfUes4+nbOfUes5 + nbOfUes6);
	seedAux << 1;
	sumoUEsSeed = somaAux.str() + "_" + seedAux.str();
	path = defaultPath + baseX + "mobility_vehs/slice_6/ns2_veh_" + sumoUEsSeed + "_slice_6.tcl";
	Ns2MobilityHelper vehMobility_3 = Ns2MobilityHelper (path);
	vehMobility_3.DefineTraceID(3);
	vehMobility_3.Install ();

	//Slice 16 - Semaforos mobilidade

	MobilityHelper trafficLightMobility;
	std::ifstream mobilityTrafficLight(defaultPath + "ns-3.29/mobility/traffic_light_position/trafficLightPosition");
	float x, y; char separator;
	Ptr<ListPositionAllocator> trafficLightPositionAlloc = CreateObject<
			ListPositionAllocator>();
	while ((mobilityTrafficLight >> x >> separator >> y) && (separator == ',')){
		trafficLightPositionAlloc->Add(Vector(x, y, 0.0));
	}
	trafficLightMobility.SetPositionAllocator(trafficLightPositionAlloc);
	trafficLightMobility.Install(ueNodes7);
	trafficLightMobility.AssignTraceID(ueNodes7, 4);

	/*1.4 MHz  - Usable PRBs = 6
	3  MHz  - Usable PRBs = 15
	5  MHz  - Usable PRBs = 25
	10  MHz  - Usable PRBs = 50
	15  MHz  - Usable PRBs = 75
	20  MHz  - Usable PRBs = 100*/
	//###Algumas configuracoes
	lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (3100)); //2655 MHz
	lteHelper->SetUeDeviceAttribute ("DlEarfcn", UintegerValue (3100)); //2655 MHz
	lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (3100 + 18000));
	Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (32.0)); //Confirmar depois
	Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (10.0));
	lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (nbRBsDownlink));
	lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (nbRBsUplink));

	// Install LTE Devices to the nodes
	NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
	NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ueNodes);
	NetDeviceContainer ueLteDevs2 = lteHelper->InstallUeDevice(ueNodes2);
	NetDeviceContainer ueLteDevs3 = lteHelper->InstallUeDevice(ueNodes3);
	NetDeviceContainer ueLteDevs4 = lteHelper->InstallUeDevice(ueNodes4);
	NetDeviceContainer ueLteDevs5 = lteHelper->InstallUeDevice(ueNodes5);
	NetDeviceContainer ueLteDevs6 = lteHelper->InstallUeDevice(ueNodes6);
	NetDeviceContainer ueLteDevs7 = lteHelper->InstallUeDevice(ueNodes7);

	//######Fading Trace Model ---> Esse que define o fading de cada slice!
	std::vector<int> traceIDs = {1, 2, 3, 4};
	std::vector<std::string> files = {"fading_trace_ETU_3kmph.fad", "fading_trace_ETU_20kmph.fad", "fading_trace_ETU_40kmph.fad", "fading_trace_ETU_0kmph.fad"};
	uint32_t i_t;
	for(i_t=0; i_t<files.size(); i_t++){

		std::string traceName = files.at(i_t);
		std::string baseAddress = "src/lte/model/fading-traces/" + traceName; //Terminal
		std::string alternativeAddress = defaultPath + "ns-3.29/" + baseAddress; //IDE

		std::ifstream ifTraceFile;
		ifTraceFile.open(alternativeAddress, std::ifstream::in);
		if (ifTraceFile.good()) {
			// script launched by test.py
			lteHelper->AssignFileTraces(alternativeAddress, traceIDs.at(i_t));
		} else {
			// script launched as an example
			lteHelper->AssignFileTraces(baseAddress, traceIDs.at(i_t));
		}
	}

	//#####################

	//SDN --- Na verdade eh so um! Mas depois tem que refatorar o codigo
	Ptr<SDNController> sdnController = Create<SDNController>(
			Create<SliceOrchestrator>());
	Ptr<SliceOrchestrator> sliceOrchestrator =
			sdnController->GetSliceOrchestrator();
	Ptr<SDNController> uplinkSDNController = Create<SDNController>(
			Create<SliceOrchestrator>());
	Ptr<SliceOrchestrator> uplinkSliceOrchestrator =
			uplinkSDNController->GetSliceOrchestrator();
	uint32_t iUe;
	vector<int> imsis1;
	for (iUe = 0; iUe < ueLteDevs.GetN(); iUe++) {
		Ptr<NetDevice> ue = ueLteDevs.Get(iUe);
		Ptr<LteUeNetDevice> ueLteDevice = ue->GetObject<LteUeNetDevice>();
		imsis1.push_back(ueLteDevice->GetImsi());
	}

	vector<int> imsis2;
	for (iUe = 0; iUe < ueLteDevs2.GetN(); iUe++) {
		Ptr<NetDevice> ue = ueLteDevs2.Get(iUe);
		Ptr<LteUeNetDevice> ueLteDevice2 = ue->GetObject<LteUeNetDevice>();
		imsis2.push_back(ueLteDevice2->GetImsi());
	}
	vector<int> imsis3;
	for (iUe = 0; iUe < ueLteDevs3.GetN(); iUe++) {
		Ptr<NetDevice> ue = ueLteDevs3.Get(iUe);
		Ptr<LteUeNetDevice> ueLteDevice3 = ue->GetObject<LteUeNetDevice>();
		imsis3.push_back(ueLteDevice3->GetImsi());
	}
	vector<int> imsis4;
	for (iUe = 0; iUe < ueLteDevs4.GetN(); iUe++) {
		Ptr<NetDevice> ue = ueLteDevs4.Get(iUe);
		Ptr<LteUeNetDevice> ueLteDevice4 = ue->GetObject<LteUeNetDevice>();
		imsis4.push_back(ueLteDevice4->GetImsi());
	}
	vector<int> imsis5;
	for (iUe = 0; iUe < ueLteDevs5.GetN(); iUe++) {
		Ptr<NetDevice> ue = ueLteDevs5.Get(iUe);
		Ptr<LteUeNetDevice> ueLteDevice5 = ue->GetObject<LteUeNetDevice>();
		imsis5.push_back(ueLteDevice5->GetImsi());
	}
	vector<int> imsis6;
	for (iUe = 0; iUe < ueLteDevs6.GetN(); iUe++) {
		Ptr<NetDevice> ue = ueLteDevs6.Get(iUe);
		Ptr<LteUeNetDevice> ueLteDevice6 = ue->GetObject<LteUeNetDevice>();
		imsis6.push_back(ueLteDevice6->GetImsi());
	}

	vector<int> imsis7;
	for (iUe = 0; iUe < ueLteDevs7.GetN(); iUe++) {
		Ptr<NetDevice> ue = ueLteDevs7.Get(iUe);
		Ptr<LteUeNetDevice> ueLteDevice7 = ue->GetObject<LteUeNetDevice>();
		imsis7.push_back(ueLteDevice7->GetImsi());
	}


	uint32_t iEnb = 0;
	int enodebID; //Só um, nesse caso
	//Fazer alocação dos RBs por Frame. Downlink
	for (iEnb = 0; iEnb < enbLteDevs.GetN(); iEnb++) {
		Ptr<NetDevice> enodeb = enbLteDevs.Get(iEnb);
		Ptr<LteEnbNetDevice> enodebLteDevice =
				enodeb->GetObject<LteEnbNetDevice>();
		Ptr<LteEnbMac> mac = enodebLteDevice->GetMac();
		Ptr<LteEnbRrc> rrc = enodebLteDevice->GetRrc();
		SlicesOptimizer* slicesOptimizer = mac->GetSlicesOptimizer();
		slicesOptimizer->SetDynamicSlicingFlag(DYNAMICSLICING);
		slicesOptimizer->SetRBGFlag(RBGGRANULARITY);
		slicesOptimizer->SetShareOptionFlag(SHARE_SLICE_BETWEEN_UEs);
		uint8_t dlBand = enodebLteDevice->GetDlBandwidth();
		int rbgSize = slicesOptimizer->GetRbgSize(
				enodebLteDevice->GetDlBandwidth());
		int totalRBGsFrame = (dlBand / rbgSize) * 10;
		int totalRBsFrame = totalRBGsFrame * 2;
		slicesOptimizer->SetRBGSize(rbgSize);
		slicesOptimizer->SetUsedRBsDownlinkTrace(downlinkTraceAddress);
		rrc->SetSliceOptimizer(slicesOptimizer);
		Ptr<FfMacScheduler> macScheduler = enodebLteDevice->GetMacScheduler();
		macScheduler->SetSlicesOptimizer(slicesOptimizer);
		enodebID = enodeb->GetNode()->GetId();
		sliceOrchestrator->ReceiveSliceOptimizer(enodebID, totalRBsFrame, dlBand,
				slicesOptimizer);
	}

	//Fazer alocação dos RBs por Frame. Uplink
	for (iEnb = 0; iEnb < enbLteDevs.GetN(); iEnb++) {
		Ptr<NetDevice> enodeb = enbLteDevs.Get(iEnb);
		Ptr<LteEnbNetDevice> enodebLteDevice =
				enodeb->GetObject<LteEnbNetDevice>();
		Ptr<LteEnbMac> mac = enodebLteDevice->GetMac();
		Ptr<LteEnbRrc> rrc = enodebLteDevice->GetRrc();
		SlicesOptimizer* uplinkSlicesOptimizer = mac->GetUplinkSlicesOptimizer();
		uplinkSlicesOptimizer->SetDynamicSlicingFlag(DYNAMICSLICING);
		uplinkSlicesOptimizer->SetRBGFlag(RBGGRANULARITY);
		uplinkSlicesOptimizer->SetShareOptionFlag(SHARE_SLICE_BETWEEN_UEs);
		uplinkSlicesOptimizer->SetFirstSlice(4); //Primeiro Slice, usado para contagem.
		uplinkSlicesOptimizer->SetNumberOfUEs(nbOfUes + nbOfUes2 +  nbOfUes3 + nbOfUes4 + nbOfUes5 + nbOfUes7);
		uint8_t ulBand = enodebLteDevice->GetUlBandwidth();
		int rbgSize = uplinkSlicesOptimizer->GetRbgSize(
				enodebLteDevice->GetUlBandwidth());
		int totalRBGsFrame = (ulBand / rbgSize) * 10; // Depois dar uma limpada ....
		int totalRBsFrame = totalRBGsFrame * 2;
		uplinkSlicesOptimizer->SetNbRBGs(nbRBGsSubframeUplink);
		uplinkSlicesOptimizer->SetUsedRBsUplinkTrace(uplinkTraceAddress);
		rrc->SetUplinkSliceOptimizer(uplinkSlicesOptimizer);
		Ptr<FfMacScheduler> macScheduler = enodebLteDevice->GetMacScheduler();
		macScheduler->SetUplinkSlicesOptimizer(uplinkSlicesOptimizer);
		enodebID = enodeb->GetNode()->GetId();
		uplinkSliceOrchestrator->ReceiveSliceOptimizer(enodebID, totalRBsFrame, ulBand,
				uplinkSlicesOptimizer);
	}

	/*sdnController->ReceiveServiceDescriptors(enodebID, 5, imsis1, 1);
	sdnController->ReceiveServiceDescriptors(enodebID, 1, imsis2, 2);
	sdnController->ReceiveServiceDescriptors(enodebID, 2, imsis4, 3);
	sdnController->ReceiveServiceDescriptors(enodebID, 4, imsis5, 4);
	sdnController->ReceiveServiceDescriptors(enodebID, 3, imsis6, 5);*/
	sdnController->ReceiveServiceDescriptors(enodebID, 4, imsis1, 1);
	sdnController->ReceiveServiceDescriptors(enodebID, 1, imsis2, 2);
	sdnController->ReceiveServiceDescriptors(enodebID, 2, imsis4, 3);
	sdnController->ReceiveServiceDescriptors(enodebID, 3, imsis5, 4);
	sdnController->ReceiveServiceDescriptors(enodebID, 5, imsis6, 5);
	sliceOrchestrator->SendSlices(enodebID);
	sliceOrchestrator->SendSlicingMatrix(enodebID, "downlink", nbRBGsSubframeDownlink,
			defaultPath + "ns-3.29/matrix_slicing_definition/downlinkMatrix20MHz");

	/*uplinkSDNController->ReceiveServiceDescriptors(enodebID, 6, imsis1, 11);
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 1, imsis2, 12);
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 3, imsis3, 13);
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 2, imsis4, 14);
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 5, imsis5, 15);
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 4, imsis7, 16);*/
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 3, imsis1, 11);
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 1, imsis2, 12);
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 5, imsis3, 13);
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 2, imsis4, 14);
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 4, imsis5, 15);
	uplinkSDNController->ReceiveServiceDescriptors(enodebID, 6, imsis7, 16);
	uplinkSliceOrchestrator->SendSlices(enodebID);
	uplinkSliceOrchestrator->SendSlicingMatrix(enodebID, "uplink", nbRBGsSubframeUplink,
			defaultPath + "ns-3.29/matrix_slicing_definition/uplinkMatrix20MHz");

	// Install the IP stack on the UEs
	internet.Install(ueNodes);
	internet.Install(ueNodes2);
	internet.Install(ueNodes3);
	internet.Install(ueNodes4);
	internet.Install(ueNodes5);
	internet.Install(ueNodes6);
	internet.Install(ueNodes7);
	Ipv4InterfaceContainer ueIpIface, ueIpIface2, ueIpIface3, ueIpIface4, ueIpIface5, ueIpIface6, ueIpIface7;
	ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs));
	ueIpIface2 = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs2));
	ueIpIface3 = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs3));
	ueIpIface4 = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs4));
	ueIpIface5 = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs5));
	ueIpIface6 = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs6));
	ueIpIface7 = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevs7));
	// Assign IP address to UEs, and install applications
	lteHelper->Attach(ueLteDevs, enbLteDevs.Get(0));
	lteHelper->Attach(ueLteDevs2, enbLteDevs.Get(0));
	lteHelper->Attach(ueLteDevs3, enbLteDevs.Get(0));
	lteHelper->Attach(ueLteDevs4, enbLteDevs.Get(0));
	lteHelper->Attach(ueLteDevs5, enbLteDevs.Get(0));
	lteHelper->Attach(ueLteDevs6, enbLteDevs.Get(0));
	lteHelper->Attach(ueLteDevs7, enbLteDevs.Get(0));

	for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
		Ptr<Node> ueNode = ueNodes.Get(u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting =
				ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
		ueStaticRouting->SetDefaultRoute(
				epcHelper->GetUeDefaultGatewayAddress(), 1);
		//Bearer é configurado abaixo
		Ptr<NetDevice> netDevice = ueLteDevs.Get(u);
		Ptr<LteUeNetDevice> ueLteDevice =
				netDevice->GetObject<LteUeNetDevice>();
		epcHelper->ActivateEpsBearer(netDevice, ueLteDevice->GetImsi(),
				EpcTft::Default(), EpsBearer(EpsBearer::GBR_CONV_VOICE)); //QCI 1
	}

	for (uint32_t u = 0; u < ueNodes2.GetN(); ++u) {
		Ptr<Node> ueNode2 = ueNodes2.Get(u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting2 =
				ipv4RoutingHelper.GetStaticRouting(ueNode2->GetObject<Ipv4>());
		ueStaticRouting2->SetDefaultRoute(
				epcHelper->GetUeDefaultGatewayAddress(), 1);
		//Bearer é configurado abaixo
		Ptr<NetDevice> netDevice = ueLteDevs2.Get(u);
		Ptr<LteUeNetDevice> ueLteDevice =
				netDevice->GetObject<LteUeNetDevice>();
		epcHelper->ActivateEpsBearer(netDevice, ueLteDevice->GetImsi(),
				EpcTft::Default(),
				EpsBearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT)); //QCI 9
	}

	for (uint32_t u = 0; u < ueNodes3.GetN(); ++u) {
		Ptr<Node> ueNode3 = ueNodes3.Get(u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting3 =
				ipv4RoutingHelper.GetStaticRouting(ueNode3->GetObject<Ipv4>());
		ueStaticRouting3->SetDefaultRoute(
				epcHelper->GetUeDefaultGatewayAddress(), 1);
		//Bearer é configurado abaixo
		Ptr<NetDevice> netDevice = ueLteDevs3.Get(u);
		Ptr<LteUeNetDevice> ueLteDevice =
				netDevice->GetObject<LteUeNetDevice>();
		epcHelper->ActivateEpsBearer(netDevice, ueLteDevice->GetImsi(),
				EpcTft::Default(), EpsBearer(EpsBearer::GBR_NON_CONV_VIDEO)); //QCI 4
	}

	for (uint32_t u = 0; u < ueNodes4.GetN(); ++u) {
		Ptr<Node> ueNode4 = ueNodes4.Get(u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting4 =
				ipv4RoutingHelper.GetStaticRouting(ueNode4->GetObject<Ipv4>());
		ueStaticRouting4->SetDefaultRoute(
				epcHelper->GetUeDefaultGatewayAddress(), 1);
		//Bearer é configurado abaixo
		Ptr<NetDevice> netDevice = ueLteDevs4.Get(u);
		Ptr<LteUeNetDevice> ueLteDevice =
				netDevice->GetObject<LteUeNetDevice>();
		epcHelper->ActivateEpsBearer(netDevice, ueLteDevice->GetImsi(),
				EpcTft::Default(), EpsBearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT)); //QCI 9
	}

	for (uint32_t u = 0; u < ueNodes5.GetN(); ++u) {
		Ptr<Node> ueNode5 = ueNodes5.Get(u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting5 =
				ipv4RoutingHelper.GetStaticRouting(ueNode5->GetObject<Ipv4>());
		ueStaticRouting5->SetDefaultRoute(
				epcHelper->GetUeDefaultGatewayAddress(), 1);
		//Bearer é configurado abaixo
		Ptr<NetDevice> netDevice = ueLteDevs5.Get(u);
		Ptr<LteUeNetDevice> ueLteDevice =
				netDevice->GetObject<LteUeNetDevice>();
		epcHelper->ActivateEpsBearer(netDevice, ueLteDevice->GetImsi(),
				EpcTft::Default(), EpsBearer(EpsBearer::GBR_CONV_VOICE)); //QCI 1
	}

	for (uint32_t u = 0; u < ueNodes6.GetN(); ++u) {
		Ptr<Node> ueNode6 = ueNodes6.Get(u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting6 =
				ipv4RoutingHelper.GetStaticRouting(ueNode6->GetObject<Ipv4>());
		ueStaticRouting6->SetDefaultRoute(
				epcHelper->GetUeDefaultGatewayAddress(), 1);
		//Bearer é configurado abaixo
		Ptr<NetDevice> netDevice = ueLteDevs6.Get(u);
		Ptr<LteUeNetDevice> ueLteDevice =
				netDevice->GetObject<LteUeNetDevice>();
		epcHelper->ActivateEpsBearer(netDevice, ueLteDevice->GetImsi(),
				EpcTft::Default(), EpsBearer(EpsBearer::GBR_NON_CONV_VIDEO)); //QCI 4
	}

	for (uint32_t u = 0; u < ueNodes7.GetN(); ++u) {
		Ptr<Node> ueNode7 = ueNodes7.Get(u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting7 =
				ipv4RoutingHelper.GetStaticRouting(ueNode7->GetObject<Ipv4>());
		ueStaticRouting7->SetDefaultRoute(
				epcHelper->GetUeDefaultGatewayAddress(), 1);
		//Bearer é configurado abaixo
		Ptr<NetDevice> netDevice = ueLteDevs7.Get(u);
		Ptr<LteUeNetDevice> ueLteDevice =
				netDevice->GetObject<LteUeNetDevice>();
		epcHelper->ActivateEpsBearer(netDevice, ueLteDevice->GetImsi(),
				EpcTft::Default(), EpsBearer(EpsBearer::GBR_NON_CONV_VIDEO)); //QCI 4
	}

	// Install and start applications on UEs and remote host
	uint16_t dlPort = 1000;
	uint16_t ulPort = 11000;
	//uint16_t otherPort = 3000;
	ApplicationContainer clientApps;
	ApplicationContainer clientApps_FTP;
	ApplicationContainer clientApps_VoiP_1_volta;
	ApplicationContainer clientApps_VoiP_1_ida;
	ApplicationContainer clientApps_VoiP_2_volta;
	ApplicationContainer clientApps_VoiP_2_ida;
	ApplicationContainer clientApps_HTTP;
	ApplicationContainer serverApps;
	ApplicationContainer serverApps_HTTP;

	//Colocarei os UEs todos no serverApps para facilitar a inicialização randomica.

//Pedestres - INICIO

	//VoIP - Volta
	for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
		//Downlink
		++dlPort;
		Ipv4Address UEAddress = ueIpIface.GetAddress(u);
		OnOffHelper onoffClientHelper("ns3::UdpSocketFactory",
				InetSocketAddress(UEAddress, dlPort));
		onoffClientHelper.SetAttribute("OnTime",
				StringValue("ns3::ExponentialRandomVariable[Mean=0.352]")); //(https://groups.google.com/forum/#!topic/ns-3-users/YNlyo3iXKss)
		onoffClientHelper.SetAttribute("OffTime",
				StringValue("ns3::ExponentialRandomVariable[Mean=0.65]"));

		onoffClientHelper.SetAttribute("DataRate", StringValue("64Kbps")); //Dado efetivo (Considera apenas o payload no cálculo)
		onoffClientHelper.SetAttribute("PacketSize", UintegerValue(160)); //Payload
		clientApps_VoiP_1_volta.Add(onoffClientHelper.Install(remoteHost));

	}

	//VoIP - IDA
	for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
		//Uplink
		++ulPort;
		OnOffHelper onoffClientHelper2("ns3::UdpSocketFactory",
				InetSocketAddress(internetIpIfaces.GetAddress(1), ulPort));
		onoffClientHelper2.SetAttribute("OnTime",
				StringValue("ns3::ExponentialRandomVariable[Mean=0.352]")); //(https://groups.google.com/forum/#!topic/ns-3-users/YNlyo3iXKss)
		onoffClientHelper2.SetAttribute("OffTime",
				StringValue("ns3::ExponentialRandomVariable[Mean=0.65]"));

		onoffClientHelper2.SetAttribute("DataRate", StringValue("64Kbps")); //Dado efetivo (Considera apenas o payload no cálculo)
		onoffClientHelper2.SetAttribute("PacketSize", UintegerValue(160)); //Payload
		// ApplicationContainer.
		clientApps_VoiP_1_ida.Add(onoffClientHelper2.Install(ueNodes.Get(u)));
	}



	//FTP
	dlPort = 2000;
	for (uint32_t u = 0; u < ueNodes2.GetN(); ++u) {  //Fazer um slice de retorno! TCP - Ida (Data) e Volta (Acknowledge)
		  ++dlPort;
		  Ipv4Address UEAddress = ueIpIface2.GetAddress(u);
		  BulkSendHelper BulkClientSendHelper ("ns3::TcpSocketFactory",
		                         InetSocketAddress (UEAddress, dlPort)); //Para quem mandar o dado.
		  // Set the amount of data to send in bytes.  Zero is unlimited.
		  BulkClientSendHelper.SetAttribute ("SendSize", UintegerValue (1000)); //Não tá mudando
		  BulkClientSendHelper.SetAttribute ("MaxBytes", UintegerValue (0)); //Infinito
		  clientApps_FTP.Add(BulkClientSendHelper.Install (remoteHost));

		  PacketSinkHelper sink ("ns3::TcpSocketFactory",
		                         InetSocketAddress (UEAddress, dlPort));
		  serverApps.Add(sink.Install (ueNodes2.Get(u)));

	}
//Pedestres - FIM
//######################################################################################################

//Ciclistas - INICIO

	ulPort = 13000;
	//Live Video Streaming
	for (uint32_t u = 0; u < ueNodes3.GetN(); ++u) {
		std::string fileName = videoByc; // Kbps
		UdpTraceClientHelper udpClient(internetIpIfaces.GetAddress(1), ulPort, fileName);
		udpClient.SetAttribute("MaxPacketSize", UintegerValue(1460));
		clientApps.Add(udpClient.Install (ueNodes3.Get(u)));
		ulPort++;
	}


//Ciclistas - FIM
//###########################################################################################

//HTTP 3GPP Web browsing
	dlPort = 3000;
	for (uint32_t u = 0; u < ueNodes4.GetN(); ++u) {  //Fazer um slice de retorno! TCP - Ida (Data) e Volta (Acknowledge)
		  ++dlPort;

		  Ipv4Address serverAddress = internetIpIfaces.GetAddress (1);

		  // Create HTTP server helper
		  ThreeGppHttpServerHelper serverHelper (serverAddress);

		  // Install HTTP server
		  ApplicationContainer serverApps = serverHelper.Install (remoteHost);
		  Ptr<ThreeGppHttpServer> httpServer = serverApps.Get (0)->GetObject<ThreeGppHttpServer> ();
		  httpServer->SetAttribute("LocalPort", UintegerValue (dlPort));
		  httpServer->SetMtuSize(1300); // Se não mudar vai dar erro. ---> assert failed. cond="m_current + delta <= m_dataEnd", file=./ns3/buffer.h, line=859


		  // Setup HTTP variables for the server
		  PointerValue varPtr;
		  httpServer->GetAttribute ("Variables", varPtr);
		  Ptr<ThreeGppHttpVariables> httpVariables = varPtr.Get<ThreeGppHttpVariables> ();
		  httpVariables->SetMainObjectSizeMean (102400); // 100kB
		  httpVariables->SetMainObjectSizeStdDev (40960); // 40kB


		  // Create HTTP client helper
		  ThreeGppHttpClientHelper clientHelper (serverAddress);

		  // Install HTTP client
		  ApplicationContainer clientApps = clientHelper.Install (ueNodes4.Get (u));
		  Ptr<ThreeGppHttpClient> httpClient = clientApps.Get (0)->GetObject<ThreeGppHttpClient> ();
		  httpClient->SetAttribute("RemoteServerPort", UintegerValue (dlPort));

		  serverApps_HTTP.Add(serverApps);
		  clientApps_HTTP.Add(clientApps);

	}

//Veiculos - INICIO

	//VoIP
	dlPort = 4000;
	//VoIP - Volta
	for (uint32_t u = 0; u < ueNodes5.GetN(); ++u) {
		//Downlink
		++dlPort;
		Ipv4Address UEAddress = ueIpIface5.GetAddress(u);
		OnOffHelper onoffClientHelper("ns3::UdpSocketFactory",
				InetSocketAddress(UEAddress, dlPort));
		onoffClientHelper.SetAttribute("OnTime",
				StringValue("ns3::ExponentialRandomVariable[Mean=0.352]")); //(https://groups.google.com/forum/#!topic/ns-3-users/YNlyo3iXKss)
		onoffClientHelper.SetAttribute("OffTime",
				StringValue("ns3::ExponentialRandomVariable[Mean=0.65]"));

		onoffClientHelper.SetAttribute("DataRate", StringValue("64Kbps")); //Dado efetivo (Considera apenas o payload no cálculo)
		onoffClientHelper.SetAttribute("PacketSize", UintegerValue(160)); //Payload
		// ApplicationContainer.
		clientApps_VoiP_2_volta.Add(onoffClientHelper.Install(remoteHost));

	}

	//VoIP - IDA
	ulPort = 14000;
	for (uint32_t u = 0; u < ueNodes5.GetN(); ++u) {
		//Uplink
		++ulPort;
		OnOffHelper onoffClientHelper2("ns3::UdpSocketFactory",
				InetSocketAddress(internetIpIfaces.GetAddress(1), ulPort));
		onoffClientHelper2.SetAttribute("OnTime",
				StringValue("ns3::ExponentialRandomVariable[Mean=0.352]")); //(https://groups.google.com/forum/#!topic/ns-3-users/YNlyo3iXKss)
		onoffClientHelper2.SetAttribute("OffTime",
				StringValue("ns3::ExponentialRandomVariable[Mean=0.65]"));

		onoffClientHelper2.SetAttribute("DataRate", StringValue("64Kbps")); //Dado efetivo (Considera apenas o payload no cálculo)
		onoffClientHelper2.SetAttribute("PacketSize", UintegerValue(160)); //Payload
		// ApplicationContainer.
		clientApps_VoiP_2_ida.Add(onoffClientHelper2.Install(ueNodes5.Get(u)));
	}


	//Video

	dlPort = 5000;
	for (uint32_t u = 0; u < ueNodes6.GetN(); ++u) {
		Ipv4Address UEAddress = ueIpIface6.GetAddress(u);
		std::string fileName = videoVeic; // Kbps
		UdpTraceClientHelper udpClient(UEAddress, dlPort, fileName);
		udpClient.SetAttribute("MaxPacketSize", UintegerValue(1460));
		clientApps.Add(udpClient.Install(remoteHost));
		dlPort++;
	}
//Veiculos - FIM

//Semaforo - INICIO

	ulPort = 15000;
	for (uint32_t u = 0; u < ueNodes7.GetN(); ++u) {
		std::string fileName = videoSemaforo;
		UdpTraceClientHelper udpClient(internetIpIfaces.GetAddress(1), ulPort, fileName);
		udpClient.SetAttribute ("MaxPacketSize", UintegerValue (1460));
		clientApps.Add(udpClient.Install (ueNodes7.Get(u)));
		ulPort++;
	}
//Semaforo - FIM

	serverApps.Start(Seconds(0)); //Se começar em 0 causa um atraso de 3s no início do TCP.
	Ptr<UniformRandomVariable> xRandom = CreateObject<UniformRandomVariable>();
	std::vector<Ptr<Application> >::const_iterator clientAppsIter;

	for (clientAppsIter = clientApps.Begin();
			clientAppsIter != clientApps.End(); ++clientAppsIter) {//Adicionei
		//Inicializar randomicamente o tempo de início.
		Ptr<Application> app = *clientAppsIter;
		double value = xRandom->GetValue(0.0, 3.0); //0 -> 3
		app->SetStartTime(Seconds(value));
	}

	for (clientAppsIter = clientApps_VoiP_1_volta.Begin();
			clientAppsIter != clientApps_VoiP_1_volta.End(); ++clientAppsIter) {
		//Inicializar randomicamente o tempo de início.
		int i_ida = 0;
		Ptr<Application> app_volta = *clientAppsIter;
		double value = xRandom->GetValue(0.0, 3.0); //0 -> 3
		app_volta->SetStartTime(Seconds(value));
		Ptr<Application> app_ida = clientApps_VoiP_1_ida.Get(i_ida);
		app_ida->SetStartTime(Seconds(value));
		i_ida++;
	}

	for (clientAppsIter = clientApps_VoiP_2_volta.Begin();
			clientAppsIter != clientApps_VoiP_2_volta.End(); ++clientAppsIter) {
		//Inicializar randomicamente o tempo de início.
		int i_ida = 0;
		Ptr<Application> app_volta = *clientAppsIter;
		double value = xRandom->GetValue(0.0, 3.0); //0 -> 3
		app_volta->SetStartTime(Seconds(value));
		Ptr<Application> app_ida = clientApps_VoiP_2_ida.Get(i_ida);
		app_ida->SetStartTime(Seconds(value));
		i_ida++;
	}

	for (clientAppsIter = clientApps_FTP.Begin();
			clientAppsIter != clientApps_FTP.End(); ++clientAppsIter) {
		//Inicializar randomicamente o tempo de início.
		Ptr<Application> app = *clientAppsIter;
		double value = xRandom->GetValue(3.0, 5.0); //3 -> 5
		app->SetStartTime(Seconds(value));
	}

	for (clientAppsIter = clientApps_HTTP.Begin();
			clientAppsIter != clientApps_HTTP.End(); ++clientAppsIter) {
		//Inicializar randomicamente o tempo de início.
		Ptr<Application> app = *clientAppsIter;
		double value = xRandom->GetValue(4.0, 8.0);
		app->SetStartTime(Seconds(value));
	}


	flowMonitor = flowHelper.Install(ueNodes);
	flowMonitor = flowHelper.Install(ueNodes2);
	flowMonitor = flowHelper.Install(ueNodes3);
	flowMonitor = flowHelper.Install(ueNodes4);
	flowMonitor = flowHelper.Install(ueNodes5);
	flowMonitor = flowHelper.Install(ueNodes6);
	flowMonitor = flowHelper.Install(ueNodes7);
	flowMonitor = flowHelper.Install(remoteHost);

	//#######################################UPLINK#################################################


	//NET ANIMATOR###
	/*AnimationInterface::SetConstantPosition (enbNodes.Get(0), radius, radius); //eNodeB
	AnimationInterface::SetConstantPosition (pgw, radius, radius); //EPC
	AnimationInterface::SetConstantPosition  (remoteHost, radius, radius); //Remote Host
	AnimationInterface anim ("anim2.xml");
	anim.SetMaxPktsPerTraceFile(100000000);*/


	flowMonitor->SetExecutionTime(simTime);
	//lteHelper->EnableTraces ();
	// Uncomment to enable PCAP tracing
	//p2ph.EnablePcapAll("lena-epc-first");
	//lteHelper->EnablePdcpTraces();
	//lteHelper->EnableRlcTraces();
	//lteHelper->EnableDlMacTraces();
	//lteHelper->EnableUlMacTraces();
	//lteHelper->EnableUlPhyTraces();
	//lteHelper->EnableDlPhyTraces();
	//lteHelper->EnableUlRxPhyTraces();
	//lteHelper->EnableUlTxPhyTraces();
	//lteHelper->EnableDlRxPhyTraces();
	//lteHelper->EnableDlTxPhyTraces();
	//lteHelper->EnableLogComponents();

	serverApps_HTTP.Stop(Seconds(httpSimTime));
	Simulator::Stop(Seconds(simTime));
	stringstream ss;
	ss << seed << "_" << sum;
	Simulator::Schedule(Seconds(40.0), &updateMatrix_1, enodebID, sliceOrchestrator, uplinkSliceOrchestrator,
			nbRBGsSubframeDownlink, nbRBGsSubframeUplink);
	Simulator::Schedule(Seconds(httpSimTime), &updateMatrix_2, enodebID, sliceOrchestrator, uplinkSliceOrchestrator,
			nbRBGsSubframeDownlink, nbRBGsSubframeUplink);


	//Monitoramento
	if(Trace_SplitMonitoringPerSecond == true){
		//Traces Especificos
		monitoringThroughputTraceAddress = defaultPath+"ns-3.29/monitoring_per_second/throughput_trace_"+tipoAlocacao+"_"+granularidade+"_"+ss.str();
		monitoringDelayTraceAddress = defaultPath+"ns-3.29/monitoring_per_second/delay_trace_"+tipoAlocacao+"_"+granularidade+"_"+ss.str();
		monitoringJitterTraceAddress = defaultPath+"ns-3.29/monitoring_per_second/jitter_trace_"+tipoAlocacao+"_"+granularidade+"_"+ss.str();
		monitoringLostPacketsTraceAddress = defaultPath+"ns-3.29/monitoring_per_second/lost_packets_trace_"+tipoAlocacao+"_"+granularidade+"_"+ss.str();
		InstantiateTraces(ss.str(), tipoAlocacao, granularidade);
		//Abrir o trace com o vi. Por algum motivo o Text Editor nao mostra tudo.
	}

	if(Trace_MonitoringPerSecond == true){
		//Trace Geral
		monitoringTraceAddress = defaultPath+"ns-3.29/monitoring_per_second/trace_"+tipoAlocacao+"_"+granularidade+"_"+ss.str();
		outfileMonitoringTrace.open(monitoringTraceAddress, ios::out);
		outfileMonitoringTrace << "\t" << "T"<< "\t" << "D" << "\t" << "J" << "\t" << "LP" << "\n";
		//Throughput, Delay, Jitter, Lost Packets
		//Abrir o trace com o vi. Por algum motivo o Text Editor nao mostra tudo.
	}

	if(Trace_MonitoringPerSecond == true || Trace_SplitMonitoringPerSecond == true){
		QoSMonitorEachSecond(&flowHelper, flowMonitor, simTime);
	}

	if(Trace_AggregateStats == true){
		std::string finalDest = tipoAlocacao+"_"+granularidade+"_"+ss.str();
		Simulator::Schedule(Seconds(simTime - 0.001), &GatherQoSStatistics, &flowHelper, flowMonitor, finalDest, simTime);
	}

	//Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
	Simulator::Run();

	/*GtkConfigStore config;
	 config.ConfigureAttributes();*/
	//std::string name = tipoAlocacao + "_" + granularidade +"_" + share_option + "_" + ss.str() + ".xml";
	//flowMonitor->SerializeToXmlFile(name, false, false);
	Simulator::Destroy();
	return 0;

}

