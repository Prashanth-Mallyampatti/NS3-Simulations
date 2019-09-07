#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("HW1");

int main (int argc, char *argv[])
{
	uint32_t stuid=0;

	CommandLine cmd;
	cmd.AddValue ("StuID", "Student ID:", stuid);
	cmd.Parse (argc, argv);
	//std::cout << "Student Id: " << stuid << std::endl;
	
/*	Time::SetResolution (Time::NS);
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
*/
	NodeContainer nodes_AC, nodes_BC, nodes_CD;
	nodes_AC.Create (2);
	nodes_BC.Create (2);
	nodes_CD.Create (2);

	uint32_t datarate = stuid % 10 + 1;
	uint32_t delay = stuid % 3 + 1;
	
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (datarate)));
	pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delay)));
//	pointToPoint.EnablePcapAll ("Hw1");

	NetDeviceContainer device_AC, device_BC, device_CD;
	device_AC = pointToPoint.Install (nodes_AC);
	device_BC = pointToPoint.Install (nodes_BC);
	device_CD = pointToPoint.Install (nodes_CD);

	InternetStackHelper stack;
	stack.Install (nodes_AC);
	stack.Install (nodes_BC);
	stack.Install (nodes_CD);

	Ipv4AddressHelper address_AC, address_BC, address_CD;
	address_AC.SetBase ("10.1.1.0", "255.255.255.0");
	address_BC.SetBase ("10.1.2.0", "255.255.255.0");
	address_CD.SetBase ("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer interface_AC = address_AC.Assign (device_AC);
	Ipv4InterfaceContainer interface_BC = address_BC.Assign (device_BC);
	Ipv4InterfaceContainer interface_CD = address_CD.Assign (device_CD);

	UdpEchoServerHelper server_D(7999), server_C(7999);
	ApplicationContainer serverApps_D = server_D.Install (nodes_CD.Get (1));
	//ApplicationContainer serverApps_C = server_C.Install (nodes_AC.Get (1));	
	serverApps_D.Start (Seconds (1.0));
	//serverApps_C.Start (Seconds (1.0));
//	serverApps_D.Stop (Seconds (3.0));

	UdpEchoClientHelper client_A (interface_AC.GetAddress (1), 7999);
	UdpEchoClientHelper client_B (interface_BC.GetAddress (1), 7999);
	UdpEchoClientHelper client_C (interface_CD.GetAddress (1), 7999);
/*
	client_A.SetAttribute ("MaxPackets", UintegerValue (10));
	client_B.SetAttribute ("MaxPackets", UintegerValue (10));
	client_C.SetAttribute ("MaxPackets", UintegerValue (10));
*/
	client_A.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	client_B.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	client_C.SetAttribute ("Interval", TimeValue (Seconds (1.0)));

	client_A.SetAttribute ("PacketSize", UintegerValue (1024));
	client_B.SetAttribute ("PacketSize", UintegerValue (1024));
	client_C.SetAttribute ("PacketSize", UintegerValue (1024));

	ApplicationContainer clientApps_A = client_A.Install (nodes_AC.Get (0));
	ApplicationContainer clientApps_B = client_B.Install (nodes_BC.Get (0));
	ApplicationContainer clientApps_C = client_C.Install (nodes_CD.Get (0));
	clientApps_A.Start (Seconds (2.0));
	clientApps_B.Start (Seconds (2.0));
	clientApps_C.Start (Seconds (2.0));

//	clientApps_A.Stop (Seconds (3.0));
//	clientApps_B.Stop (Seconds (3.0));
//	clientApps_C.Stop (Seconds (3.0));

	pointToPoint.EnablePcapAll("Hw1");

	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);

	Time::SetResolution (Time::NS);

	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
