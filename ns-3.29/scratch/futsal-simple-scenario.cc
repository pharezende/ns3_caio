/*
 * Simple Futsal Pitch Scenario.
 */

#include "ns3/applications-module.h"
#include "ns3/bulk-send-helper.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/lte-enb-rrc.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/point-to-point-module.h"
#include "futsal-simple-scenario.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FutsalSimpleScenario");



/*
 * 1 - Install the internet interface into the remote host
 * 2 - Set the pgw <-> remote host link properties
 * 3 - Install the link in the pgw and the remote host
 * 4 - Assign and print IP for both pgw and remote host
 * 5 - Set a route from remote host to the network of future UEs
 * 6 - Return created IPs
 */
Ipv4InterfaceContainer setRemoteHostPgwNetwork(Ptr<LteHelper> lteHelper,
		Ptr<PointToPointEpcHelper> epcHelper, PointToPointHelper p2ph,
		InternetStackHelper internet, Ipv4AddressHelper ipv4h,
		Ipv4StaticRoutingHelper ipv4RoutingHelper,
		NodeContainer remoteHostContainer, Ptr<Node> pgw) {
	// retrieve the remote host
	Ptr<Node> remoteHost = remoteHostContainer.Get(0);

	// install the internet interface on remote host
	internet.Install(remoteHostContainer);

	// set the link between pgw and remote host properties
	p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
	p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
	p2ph.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));

	// install the link between the pgw and the remote host
	NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);

	// set ip v4 base and mask -> first IP address starts from 1.0.0.1
	ipv4h.SetBase("1.0.0.0", "255.0.0.0");

	// give an IP address to pgw and remote host
	Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);

	// Log the IP addresses given to pgw and remote host
	NS_LOG_INFO("PGW Address: ");
	NS_LOG_INFO(internetIpIfaces.GetAddress(0));
	NS_LOG_INFO("Remote Host Address: ");
	NS_LOG_INFO(internetIpIfaces.GetAddress(1));
	NS_LOG_INFO("");

	// get remote host routing protocol
	Ptr<Ipv4StaticRouting> remoteHostStaticRouting;
	remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(
			remoteHost->GetObject<Ipv4>());

	// add a network route to the network 7.0.0.0
	remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"),
			Ipv4Mask("255.0.0.0"), 1);

	// return network ip face
	return internetIpIfaces;
}

/*
 * 1 - Turn UE nodes into UE devices
 * 2 - Install the internet interface into UE nodes
 * 3 - Set the default Route from each UE to the pgw
 * 4 - Attach each UE to the given eNodeB
 * 5 - Return created IPs
 */
Ipv4InterfaceContainer setUeNetwork(Ptr<LteHelper> lteHelper,
		Ptr<PointToPointEpcHelper> epcHelper,
		Ipv4StaticRoutingHelper ipv4RoutingHelper, InternetStackHelper internet,
		NetDeviceContainer enbLteDev, NodeContainer ueNodes,
		std::string ueName) {
	// turn the user equipement nodes into devices
	NetDeviceContainer ueDevs = lteHelper->InstallUeDevice(ueNodes);

	// install the internet interface on user equipements
	internet.Install(ueNodes);

	// give an IP address to user equipements
	Ipv4InterfaceContainer ueIpIface;
	ueIpIface = epcHelper->AssignUeIpv4Address(ueDevs);

	// set each user equipement network routing to pgw
	for (uint32_t u = 0; u < ueNodes.GetN(); ++u) {
		// print each user equipment IP
		std::string index = std::to_string(u);
		NS_LOG_INFO("Node <" + ueName + "> [" + index + "] has IP: ");
		NS_LOG_INFO(ueIpIface.GetAddress(u));

		// get one user equipement
		Ptr<Node> ueNode = ueNodes.Get(u);

		// get the user equipement routing protocol
		Ptr<Ipv4StaticRouting> ueStaticRouting;
		ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(
				ueNode->GetObject<Ipv4>());

		// set a default route to the user equipement to reach the internet.
		// used when a specific given route is not found
		// GetUeDefaultGatewayAddress() -> return the IPv4 address of the Default Gateway to be used by UEs to reach the internet
		ueStaticRouting->SetDefaultRoute(
				epcHelper->GetUeDefaultGatewayAddress(), 1);
	}

	// log blank line
	NS_LOG_INFO("");

	// Attach user equipements device to eNodeB device
	for (uint32_t i = 0; i < ueNodes.GetN(); i++) {
		// attach user equipment device to eNodeB device
		lteHelper->Attach(ueDevs.Get(i), enbLteDev.Get(0));
	}

	// return network ip face
	return ueIpIface;
}

/* Set a constant position to the given node */
void setNodeConstantPosition(double xPos, double yPos, double zPos,
		NodeContainer node, std::string nodeName) {
	// set node initial position
	Ptr<ListPositionAllocator> positionAlloc;
	positionAlloc = CreateObject<ListPositionAllocator>();
	positionAlloc->Add(Vector(xPos, yPos, zPos));

	// set the mobility model of the node
	MobilityHelper nodeMobility;
	nodeMobility.SetPositionAllocator(positionAlloc);
	nodeMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

	// install the mobility model set on the node
	nodeMobility.Install(node);

	// log position
	char* message = new char[250];
	std::sprintf(message,
			"Node <%s> has (%.2lf, %.2lf, %.2lf) constant position",
			nodeName.c_str(), xPos, yPos, zPos);
	NS_LOG_INFO(message);
}

/* Set a random position to the given node */
void setNodeRandomPosition(double xMin, double xMax, double yMin, double yMax,
		double xInitial, double yInitial, NodeContainer node,
		std::string nodeName) {
	// set node initial position
	Ptr<ListPositionAllocator> positionAlloc;
	positionAlloc = CreateObject<ListPositionAllocator>();
	positionAlloc->Add(Vector(xInitial, yInitial, 0));

	// set the mobility model of the node
	Rectangle rectangle = Rectangle(xMin, xMax, yMin, yMax);
	MobilityHelper nodeMobility;
	nodeMobility.SetPositionAllocator(positionAlloc);
	nodeMobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Mode",
			StringValue("Time"), "Time", StringValue("100ms"), "Speed",
			StringValue("ns3::ConstantRandomVariable[Constant=1.0]"), "Bounds",
			RectangleValue(rectangle));

	// install the mobility model set on the node
	nodeMobility.Install(node);

	// log position
	char* message = new char[250];
	std::sprintf(message,
			"Node <%s> has random position starting at (%.2lf, %.2lf, 0) inside rectangle: %.2lf|%.2lf|%.2lf|%.2lf",
			nodeName.c_str(), xInitial, yInitial, xMin, xMax, yMin, yMax);
	NS_LOG_INFO(message);
}

/*
 * 1 - Create a TCP packet sink/receiver
 * 2 - Install it to the receiver node and add it to server apps
 * 3 - Create a TCP packet sender
 * 4 - Set the sender attributes to work like FTP
 * 5 - Install it to the sender node and add it to client apps
 */
void setFTP(ApplicationContainer *clientApps, ApplicationContainer *serverApps,
		uint16_t port, NodeContainer senderNode, NodeContainer receiverNode,
		Ipv4Address receiverAddress) {
	// set a TCP packets sink
	PacketSinkHelper receiver("ns3::TcpSocketFactory",
			InetSocketAddress(Ipv4Address::GetAny(), port));

	// install it to the user equipement and add it to server apps
	serverApps->Add(receiver.Install(receiverNode));

	// set a TCP client to send packets to the sink
	BulkSendHelper sender("ns3::TcpSocketFactory",
			InetSocketAddress(receiverAddress, port));

	// set sender attributes
	sender.SetAttribute("SendSize", UintegerValue(1000));
	sender.SetAttribute("MaxBytes", UintegerValue(0));

	// install it to the remote host and add it to client apps
	clientApps->Add(sender.Install(senderNode));
}

/*
 * 1 - Create a UDP packet sink/receiver
 * 2 - Install it to the receiver node and add it to server apps
 * 3 - Create a UDP packet sender
 * 4 - Set the sender attributes to work like VoIP
 * 5 - Install it to the sender node and add it to client apps
 */
void setVoIP(ApplicationContainer *clientApps, ApplicationContainer *serverApps,
		uint16_t port, NodeContainer senderNode, NodeContainer receiverNode,
		Ipv4Address receiverAddress) {
	// set a UDP packets sink
	PacketSinkHelper receiver("ns3::UdpSocketFactory",
			InetSocketAddress(Ipv4Address::GetAny(), port));

	// install it to the receiver node and add it to server apps
	serverApps->Add(receiver.Install(receiverNode));

	// set a UDP client to send VoIP packets to the sink
	OnOffHelper sender("ns3::UdpSocketFactory",
			InetSocketAddress(receiverAddress, port));

	// set sender attributes
	sender.SetAttribute("OnTime",
			StringValue("ns3::ExponentialRandomVariable[Mean=0.352]"));
	sender.SetAttribute("OffTime",
			StringValue("ns3::ExponentialRandomVariable[Mean=0.65]"));
	sender.SetAttribute("DataRate", StringValue("64Kbps"));
	sender.SetAttribute("PacketSize", UintegerValue(160));

	// install it to the sender node and add it to client apps
	clientApps->Add(sender.Install(senderNode));
}

/*
 * 1 - Create a UDP packet sink/receiver
 * 2 - Install it to the receiver node and add it to server apps
 * 3 - Create a UDP packet sender with the given video trace
 * 4 - Install it to the sender node and add it to client apps
 */
void setVideoStream(ApplicationContainer *clientApps,
		ApplicationContainer *serverApps, uint16_t port,
		NodeContainer senderNode, NodeContainer receiverNode,
		Ipv4Address receiverAddress, std::string file =
				defaultPath+"ns-3.29/movies/park_cam_medium.dat") {
	// set a UDP packets sink
	PacketSinkHelper receiver("ns3::UdpSocketFactory",
			InetSocketAddress(Ipv4Address::GetAny(), port));

	// install it to the user equipement and add it to server apps
	serverApps->Add(receiver.Install(receiverNode));

	// set a UDP client trace to send packets to the sink
	UdpTraceClientHelper sender(receiverAddress, port, file);

	// set client attributes
	sender.SetAttribute("MaxPacketSize", UintegerValue(1460));

	// install it to the remote host and add it to client apps
	clientApps->Add(sender.Install(senderNode));
}




int main(int argc, char *argv[]) {

	// initialize stadium dimensions
	double stadiumMinX = 0;
	double stadiumMaxX = 70;
	double stadiumMinY = 0;
	double stadiumMaxY = 50;

	// initialize court dimensions
	double courtMinX = 15;
	double courtMaxX = 55;
	double courtMinY = 15;
	double courtMaxY = 35;

	// initialize number of spectators
	int spectators = 15;


	// initialize traces storage file
	std::string pcapPath = defaultPath+"metrics/futsal-simple-scenario";
	std::string animPath = defaultPath+"metrics/futsal-simple-scenario-animation.xml";
	std::string flowPath = defaultPath+"metrics/futsal-simple-scenario-flow.xml";

	// initialize simulation properties
	int simTime = 120000;
	bool enableTraces = false;

	//Adicionei
	//##############@Pedro##################
	int simTime_seconds = simTime/1000;
	/*##########################################
	4 Modos de Execucao:
	Estatico + Subframe: estatico subframe
	Dinamico + Subframe: dinamico subframe
	Estatico + RBG: estatico rbg
	Dinamico + RBG: dinamico rbg
	##########################################*/

	std::string tipoAlocacao = "dinamico";
	std::string granularidade = "rbg";

	bool DYNAMICSLICING = true;
	bool RBGGRANULARITY = true;
	bool SHARE_SLICE_BETWEEN_UEs = false; //Deixa como false.
	//bool BASH_ARGS = false;
	//bool Trace_USEDRBs = false;

	int nbRBsDownlink = 100;
	int nbRBGsSubframeDownlink = 25;
	int nbRBsUplink = 100;
	int nbRBGsSubframeUplink = 100;

	configurationParameters(nbRBsDownlink, nbRBsUplink);

	std::vector<int> ues_slice_1;
	std::vector<int> ues_slice_2;
	std::vector<int> ues_slice_3;
	std::vector<int> ues_slice_4;

	std::vector<int> ues_slice_11;
	std::vector<int> ues_slice_12;
	std::vector<int> ues_slice_13;
	std::vector<int> ues_slice_14;

	slicesIDToTimeDuration[1] = simTime_seconds;
	slicesIDToTimeDuration[2] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[3] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[4] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[11] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[12] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[13] = slicesIDToTimeDuration[1];
	slicesIDToTimeDuration[14] = slicesIDToTimeDuration[1];


	//Ports Numbers --- To MAP the Slices to their statistics
	//If the slices change, the method Classify in class Ipv4FlowClassifier may also need to be changed

	uint16_t transmissionPortSlice_1_11 = 1000; // Field --- Downlink e Uplink (Vale para os dois quando é TCP)
	uint16_t transmissionPortSlice_2 = 2000; // Audio
	uint16_t transmissionPortSlice_3_13 = 3000; // IMG FTP
	uint16_t transmissionPortSlice_4_14 = 4000; // TXT FTP
	uint16_t transmissionPortSlice_12 = 12000; // Video




	//############################################################

	// parse command line arguments
	CommandLine cmd;
	cmd.AddValue("stadiumMinX", "Leftmost position of the stadium",
			stadiumMinX);
	cmd.AddValue("courtMinX", "Leftmost position of the court", courtMinX);
	cmd.AddValue("courtMaxX", "Rightmost position of the court", courtMaxX);
	cmd.AddValue("stadiumMaxX", "Rightmost position of the stadium",
			stadiumMaxX);
	cmd.AddValue("stadiumMinY", "Bottom position of the stadium", stadiumMinY);
	cmd.AddValue("courtMinY", "Bottm position of the court", courtMinY);
	cmd.AddValue("courtMaxY", "Top position of the court", courtMaxY);
	cmd.AddValue("stadiumMaxY", "Top position of the stadium", stadiumMaxY);
	cmd.AddValue("spectators", "Number of spectators per sector", spectators);
	cmd.AddValue("pcapPath", "Path to store pcap traces", pcapPath);
	cmd.AddValue("animPath", "Path to store animation traces", animPath);
	cmd.AddValue("flowPath", "Path to store flow monitor traces", flowPath);
	cmd.AddValue("simTime", "Total duration of the simulation in ms", simTime);
	cmd.AddValue("enableTraces", "Enable traces", enableTraces);
	cmd.Parse(argc, argv);


	NS_LOG_INFO("========== Set Helpers ==========\n");

	// create the lte helper
	Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

	// instanciate flow monitor
	FlowMonitorHelper flowHelper;

	// create epc helper
	Ptr<PointToPointEpcHelper> epcHelper;
	epcHelper = CreateObject<PointToPointEpcHelper>();

	// create the internet interface helper
	InternetStackHelper internet;

	// Create a point to point link helper
	PointToPointHelper p2ph;

	// create ip v4 helper
	Ipv4AddressHelper ipv4h;

	// create ip v4 routing helper
	Ipv4StaticRoutingHelper ipv4RoutingHelper;

	// set epc helper
	lteHelper->SetEpcHelper(epcHelper);

	// create client and server applications
	ApplicationContainer clientApps;
	ApplicationContainer serverApps;

	NS_LOG_INFO("========== Finish Set Helpers ==========\n\n");
	NS_LOG_INFO("========== Set Remote Host and PGW ==========\n");

	// get the pwg
	Ptr<Node> pgw = epcHelper->GetPgwNode();

	// set pgw position to be constant
	setNodeConstantPosition(((stadiumMinX + stadiumMaxX) / 2) + 5, 0, 0, pgw,
			"pgw");

	// create a single remote host
	NodeContainer remoteHostNode;
	remoteHostNode.Create(1);

	// set remote host position to be constant
	setNodeConstantPosition(((stadiumMinX + stadiumMaxX) / 2) + 10, 0, 0,
			remoteHostNode, "remote host");

	// log blank line
	NS_LOG_INFO("");

	// give an IP address to pgw and remote host
	Ipv4InterfaceContainer remoteHostPgwIpIfaces;
	remoteHostPgwIpIfaces = setRemoteHostPgwNetwork(lteHelper, epcHelper, p2ph,
			internet, ipv4h, ipv4RoutingHelper, remoteHostNode, pgw);

	NS_LOG_INFO("========== Finish Set Remote Host and PGW ==========\n\n");
	NS_LOG_INFO("========== Set eNodeB ==========\n");

	// set eNodeB
	/*
	 * 1 - create one node
	 * 2 - set its position
	 * 3 - turn it into an enb device
	 */
	NodeContainer enbNode;
	enbNode.Create(1);
	setNodeConstantPosition((stadiumMinX + stadiumMaxX) / 2, 0, 0, enbNode,
			"eNodeB");
	NS_LOG_INFO("");
	NetDeviceContainer enbLteDev = lteHelper->InstallEnbDevice(enbNode);

	NS_LOG_INFO("========== Finish Set eNodeB ==========\n\n");
	NS_LOG_INFO("========== Set Ball UE ==========\n");

	// set ball UE
	/*
	 * 1 - create one node
	 * 2 - set its position
	 * 3 - attach its network
	 * 4 - make it transmit TCP packets
	 */
	NodeContainer ueBallNode;
	ueBallNode.Create(1);
	setNodeRandomPosition(courtMinX, courtMaxX, courtMinY, courtMaxY,
			(courtMinX + courtMaxX) / 2, (courtMinY + courtMaxY) / 2,
			ueBallNode, "ball");
	NS_LOG_INFO("");
	Ipv4InterfaceContainer ueBallIpFace = setUeNetwork(lteHelper, epcHelper,
			ipv4RoutingHelper, internet, enbLteDev, ueBallNode, "ball");
	setFTP(&clientApps, &serverApps, transmissionPortSlice_1_11++, ueBallNode,
			remoteHostNode, remoteHostPgwIpIfaces.GetAddress(1));

	populateSlicesWithUEs(&ues_slice_1, ueBallNode);
	populateSlicesWithUEs(&ues_slice_11, ueBallNode);

	NS_LOG_INFO("========== Finish Set Ball UE ==========\n\n");
	NS_LOG_INFO("========== Set Refree UE ==========\n");

	// create refree UE
	/*
	 * 1 - create one node
	 * 2 - set its position
	 * 3 - set its network
	 * 4 - make it transmit TCP packets
	 */
	NodeContainer ueRefreeNode;
	ueRefreeNode.Create(1);
	setNodeRandomPosition(courtMinX, courtMaxX, courtMinY, courtMaxY,
			(courtMinX + courtMaxX) / 2, (courtMinY + courtMaxY) / 2,
			ueRefreeNode, "refree");
	NS_LOG_INFO("");
	Ipv4InterfaceContainer ueRefreeIpFace = setUeNetwork(lteHelper, epcHelper,
			ipv4RoutingHelper, internet, enbLteDev, ueRefreeNode, "refree");
	setFTP(&clientApps, &serverApps, transmissionPortSlice_1_11++, remoteHostNode,
			ueRefreeNode, ueRefreeIpFace.GetAddress(0));

	populateSlicesWithUEs(&ues_slice_1, ueRefreeNode);
	populateSlicesWithUEs(&ues_slice_11, ueRefreeNode);

	NS_LOG_INFO("========== Finish Set Refree UE ==========\n\n");
	NS_LOG_INFO("========== Set Goals UEs ==========\n");

	// create goals UEs
	/*
	 * 1 - create two node, one for each goal
	 * 2 - set their position
	 * 3 - set their network
	 * 4 - make them transmit TCP packets
	 */
	NodeContainer ueGoalsNodes;
	ueGoalsNodes.Create(2);
	setNodeConstantPosition(1.1 * courtMinX, (courtMinY + courtMaxY) / 2, 0,
			ueGoalsNodes.Get(0), "goal");
	setNodeConstantPosition(0.9 * courtMaxX, (courtMinY + courtMaxY) / 2, 0,
			ueGoalsNodes.Get(1), "goal");
	NS_LOG_INFO("");
	Ipv4InterfaceContainer ueGoalsIpFace = setUeNetwork(lteHelper, epcHelper,
			ipv4RoutingHelper, internet, enbLteDev, ueGoalsNodes, "goal");
	setFTP(&clientApps, &serverApps, transmissionPortSlice_1_11++, ueGoalsNodes.Get(0),
			remoteHostNode, remoteHostPgwIpIfaces.GetAddress(1));
	setFTP(&clientApps, &serverApps, transmissionPortSlice_1_11++, ueGoalsNodes.Get(1),
			remoteHostNode, remoteHostPgwIpIfaces.GetAddress(1));

	populateSlicesWithUEs(&ues_slice_1, ueGoalsNodes);
	populateSlicesWithUEs(&ues_slice_11, ueGoalsNodes);


	NS_LOG_INFO("========== Finish Set Goals UEs ==========\n\n");

	// set spectators ratio
	double ratio = 1.0 / (spectators + 1);

	// set amount of each spectator to transmit each type of packet
	int video = std::ceil(0.15 * spectators);
	int audio = std::ceil(0.15 * spectators);
	int image = std::ceil(0.15 * spectators);

	NS_LOG_INFO("========== Set North Spectators UEs ==========\n");

	// create north spectators UEs
	/*
	 * 1 - create some nodes, one for each spectator
	 * 2 - set their position
	 * 3 - set their network
	 * 4 - make them transmit and receive packets
	 */
	NodeContainer ueNorthSpectatorsNodes;
	ueNorthSpectatorsNodes.Create(spectators);
	for (int i = 0; i < spectators; i++) {
		setNodeRandomPosition(courtMaxX, stadiumMaxX, stadiumMinY, stadiumMaxY,
				((i + 1) * ratio * (stadiumMaxX - courtMaxX)) + courtMaxX,
				((i + 1) * ratio * (stadiumMaxY - stadiumMinY)) + stadiumMinY,
				ueNorthSpectatorsNodes.Get(i), "north spectator");
	}
	NS_LOG_INFO("");
	Ipv4InterfaceContainer ueNorthSpectatorsIpFace;
	ueNorthSpectatorsIpFace = setUeNetwork(lteHelper, epcHelper,
			ipv4RoutingHelper, internet, enbLteDev, ueNorthSpectatorsNodes,
			"north spectator");
	// video transmission
	for (int i = 0; i < video; i++) {
		setVideoStream(&clientApps, &serverApps, transmissionPortSlice_12++,
				ueNorthSpectatorsNodes.Get(i), remoteHostNode,
				remoteHostPgwIpIfaces.GetAddress(1));

		populateSlicesWithUEs(&ues_slice_12, ueNorthSpectatorsNodes.Get(i));
	}
	// audio reception
	for (int i = video; i < audio + video; i++) {
		setVoIP(&clientApps, &serverApps, transmissionPortSlice_2++, remoteHostNode,
				ueNorthSpectatorsNodes.Get(i),
				ueNorthSpectatorsIpFace.GetAddress(i));

		populateSlicesWithUEs(&ues_slice_2, ueNorthSpectatorsNodes.Get(i));
	}
	// image transmission
	for (int i = audio + video; i < audio + image + video; i++) {
		setFTP(&clientApps, &serverApps, transmissionPortSlice_3_13++,
				ueNorthSpectatorsNodes.Get(i), remoteHostNode,
				remoteHostPgwIpIfaces.GetAddress(1));

		populateSlicesWithUEs(&ues_slice_3, ueNorthSpectatorsNodes.Get(i));
		populateSlicesWithUEs(&ues_slice_13, ueNorthSpectatorsNodes.Get(i));

	}
	// text reception
	for (int i = audio + image + video; i < spectators; i++) {
		setFTP(&clientApps, &serverApps, transmissionPortSlice_4_14++, remoteHostNode,
				ueNorthSpectatorsNodes.Get(i),
				ueNorthSpectatorsIpFace.GetAddress(i));

		populateSlicesWithUEs(&ues_slice_4, ueNorthSpectatorsNodes.Get(i));
		populateSlicesWithUEs(&ues_slice_14, ueNorthSpectatorsNodes.Get(i));
	}

	NS_LOG_INFO("========== Finish North Spectators UEs ==========\n\n");
	NS_LOG_INFO("========== Set South Spectators UEs ==========\n");

	// create south spectators UEs
	/*
	 * 1 - create some nodes, one for each spectator
	 * 2 - set their position
	 * 3 - set their network
	 * 4 - make them transmit and receive packets
	 */
	NodeContainer ueSouthSpectatorsNodes;
	ueSouthSpectatorsNodes.Create(spectators);
	for (int i = 0; i < spectators; i++) {
		setNodeRandomPosition(stadiumMinX, courtMinX, stadiumMinY, stadiumMaxY,
				((i + 1) * ratio * (courtMinX - stadiumMinX)) + stadiumMinX,
				((i + 1) * ratio * (stadiumMaxY - stadiumMinY)) + stadiumMinY,
				ueSouthSpectatorsNodes.Get(i), "south spectator");
	}
	NS_LOG_INFO("");
	Ipv4InterfaceContainer ueSouthSpectatorsIpFace;
	ueSouthSpectatorsIpFace = setUeNetwork(lteHelper, epcHelper,
			ipv4RoutingHelper, internet, enbLteDev, ueSouthSpectatorsNodes,
			"south spectator");
	// video transmission
	for (int i = 0; i < video; i++) {
		setVideoStream(&clientApps, &serverApps, transmissionPortSlice_12++,
				ueSouthSpectatorsNodes.Get(i), remoteHostNode,
				remoteHostPgwIpIfaces.GetAddress(1));

		populateSlicesWithUEs(&ues_slice_12, ueSouthSpectatorsNodes.Get(i));

	}
	// audio reception
	for (int i = video; i < audio + video; i++) {
		setVoIP(&clientApps, &serverApps, transmissionPortSlice_2++, remoteHostNode,
				ueSouthSpectatorsNodes.Get(i),
				ueSouthSpectatorsIpFace.GetAddress(i));

		populateSlicesWithUEs(&ues_slice_2, ueSouthSpectatorsNodes.Get(i));
	}
	// image transmission
	for (int i = audio + video; i < audio + image + video; i++) {
		setFTP(&clientApps, &serverApps, transmissionPortSlice_3_13++,
				ueSouthSpectatorsNodes.Get(i), remoteHostNode,
				remoteHostPgwIpIfaces.GetAddress(1));

		populateSlicesWithUEs(&ues_slice_3, ueSouthSpectatorsNodes.Get(i));
		populateSlicesWithUEs(&ues_slice_13, ueSouthSpectatorsNodes.Get(i));
	}
	// text reception
	for (int i = audio + image + video; i < spectators; i++) {
		setFTP(&clientApps, &serverApps, transmissionPortSlice_4_14++, remoteHostNode,
				ueSouthSpectatorsNodes.Get(i),
				ueSouthSpectatorsIpFace.GetAddress(i));

		populateSlicesWithUEs(&ues_slice_4, ueSouthSpectatorsNodes.Get(i));
		populateSlicesWithUEs(&ues_slice_14, ueSouthSpectatorsNodes.Get(i));
	}

	NS_LOG_INFO("========== Finish South Spectators UEs ==========\n\n");
	NS_LOG_INFO("========== Set East Spectators UEs ==========\n");

	// create east spectators UEs
	/*
	 * 1 - create some nodes, one for each spectator
	 * 2 - set their position
	 * 3 - set their network
	 * 4 - make them transmit and receive packets
	 */
	NodeContainer ueEastSpectatorsNodes;
	ueEastSpectatorsNodes.Create(spectators);
	for (int i = 0; i < spectators; i++) {
		setNodeRandomPosition(courtMinX, courtMaxX, stadiumMinY, courtMinY,
				((i + 1) * ratio * (courtMaxX - courtMinX)) + courtMinX,
				((i + 1) * ratio * (courtMinY - stadiumMinY)) + stadiumMinY,
				ueEastSpectatorsNodes.Get(i), "east spectator");
	}
	NS_LOG_INFO("");
	Ipv4InterfaceContainer ueEastSpectatorsIpFace;
	ueEastSpectatorsIpFace = setUeNetwork(lteHelper, epcHelper,
			ipv4RoutingHelper, internet, enbLteDev, ueEastSpectatorsNodes,
			"east spectator");
	// video transmission
	for (int i = 0; i < video; i++) {
		setVideoStream(&clientApps, &serverApps, transmissionPortSlice_12++,
				ueEastSpectatorsNodes.Get(i), remoteHostNode,
				remoteHostPgwIpIfaces.GetAddress(1));

		populateSlicesWithUEs(&ues_slice_12, ueEastSpectatorsNodes.Get(i));
	}
	// audio reception
	for (int i = video; i < audio + video; i++) {
		setVoIP(&clientApps, &serverApps, transmissionPortSlice_2++, remoteHostNode,
				ueEastSpectatorsNodes.Get(i),
				ueEastSpectatorsIpFace.GetAddress(i));

		populateSlicesWithUEs(&ues_slice_2, ueEastSpectatorsNodes.Get(i));
	}
	// image transmission
	for (int i = audio + video; i < audio + image + video; i++) {
		setFTP(&clientApps, &serverApps, transmissionPortSlice_3_13++,
				ueEastSpectatorsNodes.Get(i), remoteHostNode,
				remoteHostPgwIpIfaces.GetAddress(1));

		populateSlicesWithUEs(&ues_slice_3, ueEastSpectatorsNodes.Get(i));
		populateSlicesWithUEs(&ues_slice_13, ueEastSpectatorsNodes.Get(i));
	}
	// text reception
	for (int i = audio + image + video; i < spectators; i++) {
		setFTP(&clientApps, &serverApps, transmissionPortSlice_4_14++, remoteHostNode,
				ueEastSpectatorsNodes.Get(i),
				ueEastSpectatorsIpFace.GetAddress(i));

		populateSlicesWithUEs(&ues_slice_4, ueEastSpectatorsNodes.Get(i));
		populateSlicesWithUEs(&ues_slice_14, ueEastSpectatorsNodes.Get(i));
	}

	NS_LOG_INFO("========== Finish East Spectators UEs ==========\n\n");
	NS_LOG_INFO("========== Set West Spectators UEs ==========\n");

	// create west spectators UE
	/*
	 * 1 - create some nodes, one for each spectator
	 * 2 - set their position
	 * 3 - set their network
	 * 4 - make them transmit and receive packets
	 */
	NodeContainer ueWestSpectatorsNodes;
	ueWestSpectatorsNodes.Create(spectators);
	for (int i = 0; i < spectators; i++) {
		setNodeRandomPosition(courtMinX, courtMaxX, courtMaxY, stadiumMaxY,
				((i + 1) * ratio * (courtMaxX - courtMinX)) + courtMinX,
				((i + 1) * ratio * (stadiumMaxY - courtMaxY)) + courtMaxY,
				ueWestSpectatorsNodes.Get(i), "west spectator");
	}
	NS_LOG_INFO("");
	Ipv4InterfaceContainer ueWestSpectatorsIpFace;
	ueWestSpectatorsIpFace = setUeNetwork(lteHelper, epcHelper,
			ipv4RoutingHelper, internet, enbLteDev, ueWestSpectatorsNodes,
			"west spectator");
	// video transmission
	for (int i = 0; i < video; i++) {
		setVideoStream(&clientApps, &serverApps, transmissionPortSlice_12++,
				ueWestSpectatorsNodes.Get(i), remoteHostNode,
				remoteHostPgwIpIfaces.GetAddress(1));

		populateSlicesWithUEs(&ues_slice_12, ueWestSpectatorsNodes.Get(i));
	}
	// audio reception
	for (int i = video; i < audio + video; i++) {
		setVoIP(&clientApps, &serverApps, transmissionPortSlice_2++, remoteHostNode,
				ueWestSpectatorsNodes.Get(i),
				ueWestSpectatorsIpFace.GetAddress(i));

		populateSlicesWithUEs(&ues_slice_2, ueWestSpectatorsNodes.Get(i));
	}
	// image transmission
	for (int i = audio + video; i < audio + image + video; i++) {
		setFTP(&clientApps, &serverApps, transmissionPortSlice_3_13++,
				ueWestSpectatorsNodes.Get(i), remoteHostNode,
				remoteHostPgwIpIfaces.GetAddress(1));

		populateSlicesWithUEs(&ues_slice_3, ueWestSpectatorsNodes.Get(i));
		populateSlicesWithUEs(&ues_slice_13, ueWestSpectatorsNodes.Get(i));
	}
	// text reception
	for (int i = audio + image + video; i < spectators; i++) {
		setFTP(&clientApps, &serverApps, transmissionPortSlice_4_14++, remoteHostNode,
				ueWestSpectatorsNodes.Get(i),
				ueWestSpectatorsIpFace.GetAddress(i));

		populateSlicesWithUEs(&ues_slice_4, ueWestSpectatorsNodes.Get(i));
		populateSlicesWithUEs(&ues_slice_14, ueWestSpectatorsNodes.Get(i));
	}

	NS_LOG_INFO("========== Finish West Spectators UEs ==========\n\n");
	NS_LOG_INFO("========== Run Simulation ==========\n");

	// set server and client apps start time
	serverApps.Start(MilliSeconds(500));
	clientApps.Start(MilliSeconds(500));

	//slicing instantiation
	Ptr<SDNController> sdnControllerDownlinkPart;
	Ptr<SDNController> sdnControllerUplinkPart;
	int nbUEsUplink = 0;
	slicing(enbLteDev, DYNAMICSLICING, RBGGRANULARITY, SHARE_SLICE_BETWEEN_UEs,
			nbUEsUplink, nbRBGsSubframeDownlink, nbRBGsSubframeUplink, sdnControllerDownlinkPart, sdnControllerUplinkPart,
			ues_slice_1, ues_slice_2, ues_slice_3, ues_slice_4, ues_slice_11, ues_slice_12, ues_slice_13, ues_slice_14
			);


	// set simulation stop time
	Simulator::Stop(MilliSeconds(simTime));

	//###########@Pedro#################

	stringstream ss;
	int seed = 1;
	int nbUEs = 10;
	ss << seed << "_" << nbUEs;

	Ptr<FlowMonitor> flowMonitor;
	// install flow monitor in all nodes
	flowMonitor = flowHelper.InstallAll();


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
		QoSMonitorEachSecond(&flowHelper, flowMonitor, simTime_seconds);
	}

	if(Trace_AggregateStats == true){
		std::string finalDest = tipoAlocacao+"_"+granularidade+"_"+ss.str();
		Simulator::Schedule(Seconds(simTime_seconds - 0.001), &GatherQoSStatistics, &flowHelper, flowMonitor, finalDest, simTime_seconds);
	}


	//###################################




	// run simulation with traces if requested
	if (enableTraces) {
		// enable pcap files
		p2ph.EnablePcapAll(pcapPath);

		// enable traces
		lteHelper->EnableTraces();

		// install flow monitor in all nodes
		//flowHelper.InstallAll();

		// set animation traces recorder
		AnimationInterface anim(animPath);

		// run simulation
		Simulator::Run();
	}

	// run simulation without animation traces
	else {
		// run simulation
		Simulator::Run();
	}

	// save flow montior if requested
	if (enableTraces) {
		flowHelper.SerializeToXmlFile(flowPath, true, true);
	}

	// destroy simulation
	Simulator::Destroy();

	NS_LOG_INFO("========== Simulation Finished ==========");

	return 0;
}
