#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("HW1");

int main (int argc, char *argv[])
{
	uint32_t stuid=0;

	CommandLine cmd;
	cmd.AddValue ("StuID", "Student ID:", stuid);
	cmd.Parse (argc, argv);
	Time::SetResolution (Time::NS);
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

	NodeContainer c;
	c.Create (4);
	
	NodeContainer nodes_AC, nodes_BC, nodes_CD;
	nodes_AC = NodeContainer (c.Get (0), c.Get (2));
	nodes_BC = NodeContainer (c.Get (1), c.Get (2));
	nodes_CD = NodeContainer (c.Get (2), c.Get (3));

	InternetStackHelper stack;
	stack.Install (c);

	uint32_t datarate = stuid % 10 + 1;
	uint32_t delay = stuid % 3 + 1;
	
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue(DataRate (datarate * 1000 * 1000)));
	pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delay)));

	
	NetDeviceContainer device_AC, device_BC, device_CD;
	device_AC = pointToPoint.Install (nodes_AC);
	device_BC = pointToPoint.Install (nodes_BC);
	device_CD = pointToPoint.Install (nodes_CD);

	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interface_AC = address.Assign (device_AC);
	address.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer interface_BC = address.Assign (device_BC);
	address.SetBase ("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer interface_CD = address.Assign (device_CD);

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	UdpEchoServerHelper server_D (7999);
	ApplicationContainer serverApps_D = server_D.Install (nodes_CD.Get (1));
	serverApps_D.Start (Seconds (1.0));

	UdpEchoClientHelper client_A (interface_CD.GetAddress (1), 7999);
	UdpEchoClientHelper client_B (interface_CD.GetAddress (1), 7999);
	UdpEchoClientHelper client_C (interface_CD.GetAddress (1), 7999);
	
	client_A.SetAttribute ("MaxPackets", UintegerValue (3));
	client_B.SetAttribute ("MaxPackets", UintegerValue (3));
	client_C.SetAttribute ("MaxPackets", UintegerValue (3));
	

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

	pointToPoint.EnablePcapAll("Hw1-1");

	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
