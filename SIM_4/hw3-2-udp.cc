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
#include "iostream"

using namespace std;
using namespace ns3;

#define ONEMBPS 1000000

NS_LOG_COMPONENT_DEFINE ("ECE592HW3");

uint32_t ecmpMode = 0;
bool parse_ecmpMode(const std::string val) {
	int value = std::stoi(val);
	if (value >= 0 && value < 4) {
		ecmpMode = value;
		return true;
	}
	return false;
}

class FlowAnalyzer {
	// Step 1
	public:
		uint32_t m_totalRx = 0;
		Time m_startTime;
		Time m_lastTime;
		std::map<Ipv4Address, uint32_t> m_totalRxAll;
		std::map<Ipv4Address, Time> m_lastTimeAll;
		
		// Step 2
		FlowAnalyzer(Time val) {
			m_startTime = val;
			m_lastTime = val;
		}

		virtual ~FlowAnalyzer() {}

		// Step 3
		void RecvPkt(Ptr<const Packet> p, const Address &a) {
			// Step 3a
			InetSocketAddress thisAddr = InetSocketAddress::ConvertFrom(a);
			Ipv4Address addrstr = thisAddr.GetIpv4();
			
			// Step 3b
			Time now(Simulator::Now());
			m_lastTimeAll[addrstr] = now;
			m_lastTime = now;

			// Step 3c
			m_totalRxAll[addrstr] += (p->GetSize() * 8);
			m_totalRx += (p->GetSize() * 8);
		}

		double CalcThruPut() {
			// NS_LOG_UNCOND("ST: " << m_startTime.GetSeconds() << " ET: " << m_lastTime.GetSeconds());
			return ((m_totalRx / (m_lastTime.GetSeconds() - m_startTime.GetSeconds())) / double(ONEMBPS));
		}

		void getPerFlowThruPut()
		{
			std::map<Ipv4Address, uint32_t>::iterator it = m_totalRxAll.begin();
			while(it != m_totalRxAll.end())
			{
				NS_LOG_UNCOND(it->first << " - " << CalcThruPut(it->first));
				it++;
			}
		}
		// Step 4
		double CalcThruPut(Ipv4Address addr) {
			return ((m_totalRxAll.find(addr)->second / (m_lastTimeAll.find(addr)->second.GetSeconds() - m_startTime.GetSeconds())) / double(ONEMBPS));
		}

		void getPerFlowCT()
		{
			std::map<Ipv4Address, uint32_t>::iterator it = m_totalRxAll.begin();
			while(it != m_totalRxAll.end())
			{
					NS_LOG_UNCOND(it->first << " - " << GetFCT(it->first));
					it++;
			}
		}
		// Step 5
		double GetFCT(Ipv4Address addr) {
			return (m_lastTimeAll.find(addr)->second.GetSeconds() - m_startTime.GetSeconds());
		}

		// Step 6
		double GetAvgFCT() {
			uint32_t numFlows = 0;
			double flowTimes = 0.0;
			for (const auto &p : m_lastTimeAll) {
				numFlows++;
				flowTimes += (p.second.GetSeconds() - m_startTime.GetSeconds());
			}
			return (flowTimes / double(numFlows));
		}
};

int main (int argc, char *argv[])
{
	uint32_t n=3, f=5, delay = 2, maxBytes=100000;
 	uint64_t dataRate = 5 * ONEMBPS;

    // Step 2, 3, 4
  	CommandLine cmd;
  	cmd.AddValue("EcmpMode", "ECMP Hashing Mode", MakeCallback (&parse_ecmpMode));
  	cmd.AddValue("nPath", "Num. of Paths", n);
    cmd.AddValue("nFlow", "Num. of Flows", f);
    cmd.AddValue("MaxBytes", "Maximum bytes to transfer", maxBytes);
  	cmd.Parse (argc, argv);

  	Time::SetResolution (Time::NS);
    Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
    Config::SetDefault("ns3::Ipv4GlobalRouting::EcmpMode", UintegerValue(ecmpMode));

  	//LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  	//LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);

  	NS_LOG_INFO("Create nodes");
    NodeContainer lvl1, lvl2, lvl3, lvl4, lvl5; // Each level corresponds to a column od nodes in the topology
    lvl1.Create(f);
    lvl2.Create(1);
    lvl3.Create(n);
    lvl4.Create(1);
    lvl5.Create(1);

    NodeContainer n1_2[f], n2_3[n], n3_4[n], n4_5[1];
	for(uint32_t i = 0; i < f; i++)
	{
        n1_2[i] = NodeContainer (lvl1.Get(i), lvl2.Get(0));
	}

    for(uint32_t i = 0; i < n; i++)
	{
        n2_3[i] = NodeContainer (lvl2.Get(0), lvl3.Get(i));
	}

    for(uint32_t i = 0; i < n; i++)
	{
        n3_4[i] = NodeContainer (lvl3.Get(i), lvl4.Get(0));
	}

    n4_5[0] = NodeContainer (lvl4.Get(0), lvl5.Get(0));


    // Step 5
  	NS_LOG_INFO("Create P2P channels");
  	PointToPointHelper p2p;
  	p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  	p2p.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delay)));
  
    PointToPointHelper p2p_1;
  	p2p_1.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (dataRate * f)));
  	p2p_1.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delay)));

  	InternetStackHelper stack;
  	stack.Install (lvl1);
    stack.Install (lvl2);
    stack.Install (lvl3);
    stack.Install (lvl4);
    stack.Install (lvl5);

  	NetDeviceContainer dev_1_2[f], dev_2_3[n], dev_3_4[n], dev_4_5[1];

	Ipv4AddressGenerator::Init (Ipv4Address ("10.1.0.0"), Ipv4Mask ("/24"));
	Ipv4Address subnet_1_2[f], subnet_2_3[n], subnet_3_4[n], subnet_4_5[1];
	
	Ipv4AddressHelper address;
	Ipv4InterfaceContainer interface_1_2[f], interface_2_3[n], interface_3_4[n], interface_4_5[1];

	for(uint32_t i = 0; i < f; i++)
	{
		dev_1_2[i] = p2p.Install (n1_2[i]);
		subnet_1_2[i] = Ipv4AddressGenerator::NextNetwork (Ipv4Mask("/24"));	
		address.SetBase (subnet_1_2[i], "255.255.255.0");
		interface_1_2[i] = address.Assign (dev_1_2[i]);
	}

    for(uint32_t i = 0; i < n; i++)
	{
		dev_2_3[i] = p2p.Install (n2_3[i]);
		subnet_2_3[i] = Ipv4AddressGenerator::NextNetwork (Ipv4Mask("/24"));	
		address.SetBase (subnet_2_3[i], "255.255.255.0");
		interface_2_3[i] = address.Assign (dev_2_3[i]);
	}

    for(uint32_t i = 0; i < n; i++)
	{
		dev_3_4[i] = p2p.Install (n3_4[i]);
		subnet_3_4[i] = Ipv4AddressGenerator::NextNetwork (Ipv4Mask("/24"));	
		address.SetBase (subnet_3_4[i], "255.255.255.0");
		interface_3_4[i] = address.Assign (dev_3_4[i]);
	}

    dev_4_5[0] = p2p_1.Install (n4_5[0]);
    subnet_4_5[0] = Ipv4AddressGenerator::NextNetwork (Ipv4Mask("/24"));	
    address.SetBase (subnet_4_5[0], "255.255.255.0");
    interface_4_5[0] = address.Assign (dev_4_5[0]);

  	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Step 6
	PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 7999));

  	ApplicationContainer sinkApps = packetSinkHelper.Install (lvl5.Get (0));
  	sinkApps.Start (Seconds (0.0));
  	sinkApps.Stop (Seconds (100.0));

	AddressValue serverAddr (InetSocketAddress(interface_4_5[0].GetAddress(1), 7999));
  
    // Step 7
	OnOffHelper clientHelper ("ns3::UdpSocketFactory", Address());
  	clientHelper.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  	clientHelper.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
	clientHelper.SetAttribute ("Remote", serverAddr);
  	clientHelper.SetAttribute ("PacketSize", UintegerValue (1000));
  	clientHelper.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  	clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));

	 FlowAnalyzer* flowAnalyzer = new FlowAnalyzer(Time(1*1000*1000*1000));
	 Ptr<PacketSink> sink = StaticCast<PacketSink> (sinkApps.Get(0));
	 sink->TraceConnectWithoutContext("Rx", MakeCallback(&FlowAnalyzer::RecvPkt, flowAnalyzer));

    // Step 8
    ApplicationContainer clientApps = clientHelper.Install (lvl1.Get (0));
    for (uint32_t i=1; i<f;i++) {
        clientApps.Add (clientHelper.Install (lvl1.Get(i)));
    }
  	clientApps.Start (Seconds (1.0));
  	clientApps.Stop (Seconds (100.0));

	// Step 9
	std::string pcapPrefix = std::string("Hw3-2/All-") + std::to_string(ecmpMode) + "-" + std::to_string(n) + "-" + std::to_string(f);
	p2p.EnablePcapAll(pcapPrefix);
 	p2p_1.EnablePcapAll(pcapPrefix);

	Simulator::Run ();
	Simulator::Destroy ();

	NS_LOG_UNCOND("The calculated flow throughput is " << flowAnalyzer->CalcThruPut() << " Mbps");
	NS_LOG_UNCOND("The calculated flow completion time is " << flowAnalyzer->GetAvgFCT() << " seconds");

	NS_LOG_UNCOND("Calculated per flow throughput");
	flowAnalyzer->getPerFlowThruPut();
	NS_LOG_UNCOND("Calculated per flow completion time");
	flowAnalyzer->getPerFlowCT();

	delete flowAnalyzer;
	return 0;
}
