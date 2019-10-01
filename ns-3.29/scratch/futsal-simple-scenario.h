/*
 * Ver /ns-3.29/manual
 */

#ifndef NS_3_29_SCRATCH_FUTSAL_AUXILIARY_H_
#define NS_3_29_SCRATCH_FUTSAL_AUXILIARY_H_

using namespace ns3;

#include "ns3/flow-monitor-module.h"


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

std::string defaultPath = "/home/pedro/Documents/ns-allinone-3.29-caio/";


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
std::vector<int> slicesIDs = {1, 2, 3, 4, 11, 12, 13, 14};
std::map<int, double> slicesIDToTimeDuration;

bool Trace_MonitoringPerSecond = true;
bool Trace_SplitMonitoringPerSecond = true;
bool Trace_AggregateStats = true;


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


void configurationParameters(int nbRBsDownlink, int nbRBsUplink){

	Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (32.0));
	Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (10.0));
	Config::SetDefault ("ns3::LteUeNetDevice::DlEarfcn", UintegerValue (3100)); //2655 MHz
	Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue (3100)); //2655 MHz
	Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", UintegerValue (3100 + 18000));
	Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (nbRBsDownlink));
	Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (nbRBsUplink));
	// set srs periodicity to 320
	Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(320));
	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpVegas"));

}


void slicing(NetDeviceContainer enbLteDevs, bool DYNAMICSLICING,
		bool RBGGRANULARITY, bool SHARE_SLICE_BETWEEN_UEs, int nbUEsUplink,
		int nbRBGsSubframeDownlink, int nbRBGsSubframeUplink,
		Ptr<SDNController> sdnControllerDownlinkPart,
		Ptr<SDNController> sdnControllerUplinkPart,
		std::vector<int> ues_slice_1, std::vector<int> ues_slice_2,
		std::vector<int> ues_slice_3, std::vector<int> ues_slice_4,
		std::vector<int> ues_slice_11, std::vector<int> ues_slice_12,
		std::vector<int> ues_slice_13, std::vector<int> ues_slice_14
		) {

	//##############Slices Description#################
	//Field
	int fieldSliceDownlinkID = 1; //Downlink ---> Referee + Ball + Goal
	int fieldSliceDownlinkPriority = 3;

	int fieldSliceUplinkID = 11; //Uplink ----> Referee + Ball + Goal
	int fieldSliceUplinkPriority = 3;
	//####

	//Video
	int videoSpectatorSliceUplinkID = 12;
	int videoSpectatorSliceUplinkPriority = 2;
	//##########

	//Audio
	int audioSpectatorSliceDownlinkID = 2;
	int audioSpectatorSliceDownlinkPriority = 2;
	//##########

	//IMG FTP
	int imgFTPSliceDownlinkID = 3;
	int imgFTPSliceDownlinkPriority = 1;

	int imgFTPSliceUplinkID = 13;
	int imgFTPSliceUplinkPriority = 1;
	//###########

	//TXT FTP
	int txtFTPSliceDownlinkID = 4;
	int txtFTPSliceDownlinkPriority = 0;

	int txtFTPSliceUplinkID = 14;
	int txtFTPSliceUplinkPriority = 0;
	//#########

	//###############################

	sdnControllerDownlinkPart = Create<SDNController>(
			Create<SliceOrchestrator>());
	Ptr<SliceOrchestrator> downlinkOrchestrator =
			sdnControllerDownlinkPart->GetSliceOrchestrator();

	sdnControllerUplinkPart = Create<SDNController>(
			Create<SliceOrchestrator>());
	Ptr<SliceOrchestrator> uplinkSliceOrchestrator =
			sdnControllerUplinkPart->GetSliceOrchestrator();

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
		//slicesOptimizer->SetUsedRBsDownlinkTrace(downlinkTraceAddress);
		rrc->SetSliceOptimizer(slicesOptimizer);
		Ptr<FfMacScheduler> macScheduler = enodebLteDevice->GetMacScheduler();
		macScheduler->SetSlicesOptimizer(slicesOptimizer);
		enodebID = enodeb->GetNode()->GetId();
		downlinkOrchestrator->ReceiveSliceOptimizer(enodebID, totalRBsFrame,
				dlBand, slicesOptimizer);
	}

	//Functions' input arguments: #1 enodeB ID; Priority; UEs in each slice; Slice ID
	sdnControllerDownlinkPart->ReceiveServiceDescriptors(enodebID,
			fieldSliceDownlinkPriority, ues_slice_1, fieldSliceDownlinkID);
	sdnControllerDownlinkPart->ReceiveServiceDescriptors(enodebID,
			audioSpectatorSliceDownlinkPriority, ues_slice_2,
			audioSpectatorSliceDownlinkID);
	sdnControllerDownlinkPart->ReceiveServiceDescriptors(enodebID,
			imgFTPSliceDownlinkPriority, ues_slice_3, imgFTPSliceDownlinkID);
	sdnControllerDownlinkPart->ReceiveServiceDescriptors(enodebID,
			txtFTPSliceDownlinkPriority, ues_slice_4, txtFTPSliceDownlinkID);

	downlinkOrchestrator->SendSlicingMatrix(enodebID, "downlink",
			nbRBGsSubframeDownlink,
			defaultPath + "ns-3.29/matrix_slicing_definition/downlinkMatrix20MHz");
	downlinkOrchestrator->SendSlices(enodebID);

	//Fazer alocação dos RBs por Frame. Uplink
	for (iEnb = 0; iEnb < enbLteDevs.GetN(); iEnb++) {
		Ptr<NetDevice> enodeb = enbLteDevs.Get(iEnb);
		Ptr<LteEnbNetDevice> enodebLteDevice =
				enodeb->GetObject<LteEnbNetDevice>();
		Ptr<LteEnbMac> mac = enodebLteDevice->GetMac();
		Ptr<LteEnbRrc> rrc = enodebLteDevice->GetRrc();
		SlicesOptimizer* uplinkSlicesOptimizer =
				mac->GetUplinkSlicesOptimizer();
		uplinkSlicesOptimizer->SetDynamicSlicingFlag(DYNAMICSLICING);
		uplinkSlicesOptimizer->SetRBGFlag(RBGGRANULARITY);
		uplinkSlicesOptimizer->SetShareOptionFlag(SHARE_SLICE_BETWEEN_UEs);
		uplinkSlicesOptimizer->SetFirstSlice(4); //Primeiro Slice, usado para contagem.
		uplinkSlicesOptimizer->SetNumberOfUEs(nbUEsUplink); //Nao conta do downlink
		uint8_t ulBand = enodebLteDevice->GetUlBandwidth();
		int rbgSize = uplinkSlicesOptimizer->GetRbgSize(
				enodebLteDevice->GetUlBandwidth());
		int totalRBGsFrame = (ulBand / rbgSize) * 10; // Depois dar uma limpada ....
		int totalRBsFrame = totalRBGsFrame * 2;
		uplinkSlicesOptimizer->SetNbRBGs(nbRBGsSubframeUplink);
		//uplinkSlicesOptimizer->SetUsedRBsUplinkTrace(uplinkTraceAddress);
		rrc->SetUplinkSliceOptimizer(uplinkSlicesOptimizer);
		Ptr<FfMacScheduler> macScheduler = enodebLteDevice->GetMacScheduler();
		macScheduler->SetUplinkSlicesOptimizer(uplinkSlicesOptimizer);
		enodebID = enodeb->GetNode()->GetId();
		uplinkSliceOrchestrator->ReceiveSliceOptimizer(enodebID, totalRBsFrame,
				ulBand, uplinkSlicesOptimizer);
	}

	sdnControllerUplinkPart->ReceiveServiceDescriptors(enodebID,
			fieldSliceUplinkPriority, ues_slice_11, fieldSliceUplinkID);
	sdnControllerUplinkPart->ReceiveServiceDescriptors(enodebID,
			videoSpectatorSliceUplinkPriority, ues_slice_12,
			videoSpectatorSliceUplinkID);
	sdnControllerUplinkPart->ReceiveServiceDescriptors(enodebID,
			imgFTPSliceUplinkPriority, ues_slice_13, imgFTPSliceUplinkID);
	sdnControllerUplinkPart->ReceiveServiceDescriptors(enodebID,
			txtFTPSliceUplinkPriority, ues_slice_14, txtFTPSliceUplinkID);

	uplinkSliceOrchestrator->SendSlicingMatrix(enodebID, "uplink",
			nbRBGsSubframeUplink,
			defaultPath + "ns-3.29/matrix_slicing_definition/uplinkMatrix20MHz");

	uplinkSliceOrchestrator->SendSlices(enodebID);
}


void populateSlicesWithUEs(std::vector<int>* ues_slice,
		NodeContainer nodeContainer) {

	uint32_t nNodes = nodeContainer.GetN();

	for (uint32_t i = 0; i < nNodes; ++i) {
		Ptr<Node> node = nodeContainer.Get(i);
		Ptr<NetDevice> netDevice = node->GetDevice(0);
		Ptr<LteUeNetDevice> lteUENetDevice =
				netDevice->GetObject<LteUeNetDevice>();
		ues_slice->push_back(lteUENetDevice->GetImsi());
	}
}

void populateSlicesWithUEs(std::vector<int>* ues_slice, Ptr<Node> node) {

	Ptr<NetDevice> netDevice = node->GetDevice(0);
	Ptr<LteUeNetDevice> lteUENetDevice = netDevice->GetObject<LteUeNetDevice>();
	ues_slice->push_back(lteUENetDevice->GetImsi());

}




#endif /* NS_3_29_SCRATCH_FUTSAL_AUXILIARY_H_ */
