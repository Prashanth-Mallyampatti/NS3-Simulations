/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include <memory>
#include <time.h>

using namespace ns3;

#define ONEMBPS 1000000

NS_LOG_COMPONENT_DEFINE ("ECE592HW2");

uint32_t g_nSource = 3;
bool parse_nSource(const std::string val) {
	int value = std::stoi(val);
	if (value > 0 && value < 4) {
		g_nSource = value;
		return true;
	}
	return false;
}

class ThruPutAnalyzer {
	// Step 4a
	public:
		uint32_t m_totalRx = 0;
		Time m_startTime;
		Time m_lastTime;
		
		ThruPutAnalyzer(Time val) {
			m_startTime = val;
			m_lastTime = val;
		}

		virtual ~ThruPutAnalyzer() {}

		// Step 4b
		void RecvPkt(Ptr<const Packet> p, const Address &a) {
			m_totalRx += (p->GetSize() * 8);
			m_lastTime = Simulator::Now();
		}

		// Step 4c
		double CalcThruPut() {
			NS_LOG_UNCOND("Total: " << m_totalRx << " start: " << m_startTime.GetSeconds() << " end: "<<m_lastTime.GetSeconds());
			return ((m_totalRx / (m_lastTime.GetSeconds() - m_startTime.GetSeconds())) / double(ONEMBPS));
		}
};

int
main (int argc, char *argv[])
{
	uint32_t stuID = 0, delay;
 	uint64_t dataRate;

   	// Step 2a, 2b
  	CommandLine cmd;
  	cmd.AddValue("StuID", "Student ID number", stuID);
  	cmd.AddValue("nSource", "Num. of Sources", MakeCallback (&parse_nSource));
  	cmd.Parse (argc, argv);

  	dataRate = ((stuID % 10) + 1) * ONEMBPS;
  	delay = ((stuID %3) +1);

  	NS_LOG_UNCOND("Data rate: " << dataRate << " bps");
  	NS_LOG_UNCOND("Delay: " << delay << " ms");

  	Time::SetResolution (Time::NS);

	// Step 3d
  	LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  	LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);

  	NS_LOG_INFO("Create nodes");
  	Ptr<Node> nodeA = CreateObject<Node> ();
  	Ptr<Node> nodeB = CreateObject<Node> ();
  	Ptr<Node> nodeC = CreateObject<Node> ();
  	Ptr<Node> nodeD = CreateObject<Node> ();

  	NodeContainer netAC (nodeA, nodeC);
  	NodeContainer netBC (nodeB, nodeC);
  	NodeContainer netCD (nodeC, nodeD);

  	NodeContainer nodes (nodeA, nodeB, nodeC, nodeD);

  	NS_LOG_INFO("Create P2P channels");
  	PointToPointHelper p2p;
  	p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  	p2p.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delay)));
  
  	NetDeviceContainer netDevAC = p2p.Install(netAC);
  	NetDeviceContainer netDevBC = p2p.Install(netBC);
  	NetDeviceContainer netDevCD = p2p.Install(netCD);

  	InternetStackHelper stack;
  	stack.Install (nodes);

  	Ipv4AddressHelper address;
  	address.SetBase ("10.1.1.0", "255.255.255.0");
  	Ipv4InterfaceContainer ifAC = address.Assign (netDevAC);
  	address.SetBase ("10.1.2.0", "255.255.255.0");
  	Ipv4InterfaceContainer ifBC = address.Assign (netDevBC);
  	address.SetBase ("10.1.3.0", "255.255.255.0");
  	Ipv4InterfaceContainer ifCD = address.Assign (netDevCD);

  	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	// Step 3a
	PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 7999));

  	ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (3));
  	sinkApps.Start (Seconds (1.0));
  	sinkApps.Stop (Seconds (10.0));

	// Step 3b
	AddressValue serverAddr (InetSocketAddress(ifCD.GetAddress(1), 7999));
  
	OnOffHelper clientHelper ("ns3::UdpSocketFactory", Address());
  	clientHelper.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  	clientHelper.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
	clientHelper.SetAttribute ("Remote", serverAddr);
  	clientHelper.SetAttribute ("PacketSize", UintegerValue (1000));
  	clientHelper.SetAttribute ("MaxBytes", UintegerValue (10000));
  	clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate/2)));

	// Step 5
	ThruPutAnalyzer* tpAnalyzer = new ThruPutAnalyzer(Time(2*1000*1000*1000));
	Ptr<PacketSink> sink = StaticCast<PacketSink> (sinkApps.Get(0));
	sink->TraceConnectWithoutContext("Rx", MakeCallback(&ThruPutAnalyzer::RecvPkt, tpAnalyzer));

	// Step 6
	ApplicationContainer clientApps = clientHelper.Install (nodes.Get (0));
  	if (g_nSource>1)
		clientApps.Add (clientHelper.Install (nodes.Get(1)));
  	if (g_nSource>2)
		clientApps.Add (clientHelper.Install (nodes.Get(2)));
  	clientApps.Start (Seconds (2.0));
  	clientApps.Stop (Seconds (10.0));

	// Step 3c
	p2p.EnablePcapAll("Hw2-2");
 
 		
	Simulator::Run ();
	Simulator::Destroy ();
  
	NS_LOG_UNCOND("The calculated throughput with " << g_nSource << " senders is " << tpAnalyzer->CalcThruPut() << " Mbps");

	delete tpAnalyzer;
	return 0;
}
