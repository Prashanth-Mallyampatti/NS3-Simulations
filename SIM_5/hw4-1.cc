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
#include <iostream>
#include <vector>
#include "ns3/traffic-control-module.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/fifo-queue-disc.h"

using namespace std;

using namespace ns3;

#define ONEMBPS 1000000

NS_LOG_COMPONENT_DEFINE ("ECE592HW4");

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
	public:
		uint32_t m_totalRx = 0;
		Time m_startTime;
		Time m_lastTime;
		std::map<Ipv4Address, uint32_t> m_totalRxAll;
		std::map<Ipv4Address, Time> m_lastTimeAll;
		
		FlowAnalyzer(Time val) {
			m_startTime = val;
			m_lastTime = val;
		}

		virtual ~FlowAnalyzer() {}

		void RecvPkt(Ptr<const Packet> p, const Address &a) {
			InetSocketAddress thisAddr = InetSocketAddress::ConvertFrom(a);
			Ipv4Address addrstr = thisAddr.GetIpv4();
			
			m_lastTimeAll[addrstr] = Simulator::Now();
			m_lastTime = Simulator::Now();

			m_totalRxAll[addrstr] += (p->GetSize() * 8);
			m_totalRx += (p->GetSize() * 8);
		}

		double CalcThruPut() {
			return ((m_totalRx / (m_lastTime.GetSeconds() - m_startTime.GetSeconds())) / double(ONEMBPS));
		}

		void getPerFlowThruPut()
		{
				std::map<Ipv4Address, uint32_t>::iterator it = m_totalRxAll.begin();
				while(it != m_totalRxAll.end())
				{
						NS_LOG_UNCOND(it->first << "-" << CalcThruPut(it->first));
						it++;
				}
		}

		double CalcThruPut(Ipv4Address addr) {
			return ((m_totalRxAll.find(addr)->second / (m_lastTimeAll.find(addr)->second.GetSeconds() - m_startTime.GetSeconds())) / double(ONEMBPS));
		}

		void getPerFlowCT()
		{
				std::map<Ipv4Address, uint32_t>::iterator it = m_totalRxAll.begin();
				while(it != m_totalRxAll.end())
				{
						NS_LOG_UNCOND(it->first << "-" << GetFCT(it->first));
						it++;
				}
		}

		double GetFCT(Ipv4Address addr) {
			return (m_lastTimeAll.find(addr)->second.GetSeconds() - m_startTime.GetSeconds());
		}

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

// Step 4
class QueueMonitor {
	// Step 4a
	public:
		Time					m_startTime;
		std::vector<uint32_t>	m_qLens;
		std::vector<Time>		m_qTimes;

		// Step 4b
		QueueMonitor(Time val) {
			m_startTime = val;
		}

		virtual ~QueueMonitor() {}

		// Step 4c
		void QueueChange(uint32_t oval, uint32_t nval) {
			m_qLens.push_back(nval);
			m_qTimes.push_back(Simulator::Now());
		}

		// Step 4d
		// Time average means you multiply each queue length with the time that the queue stays with that length, and then finally divide the sum over the course of the entire time.
		double GetAvgQueueLen() {
			// If queue is not used at all, return 0
			if (m_qLens.size() == 0) return (double)(0.0);
			double sumLensTimeAvg;
			for (uint32_t i=0; i < m_qLens.size(); i++) { 
    			double timeInterval;
				if (i==0)
					timeInterval = m_qTimes.at(0).GetSeconds() - m_startTime.GetSeconds();
				else
					timeInterval = m_qTimes.at(i).GetSeconds() - m_qTimes.at(i-1).GetSeconds();
				sumLensTimeAvg += m_qLens.at(i) * timeInterval;
			}
			return (double) (sumLensTimeAvg / (double) (m_qTimes.at(m_qTimes.size() -1).GetSeconds() - m_startTime.GetSeconds()));
		}

		// Step 4e
		void SaveQueueLen(std::string fn) {
			std::fstream outfile;
    		outfile.open(fn,ios::trunc | ios::out | ios::in );

			for (uint32_t i = 0; i < m_qLens.size(); i++) {
				outfile << m_qTimes.at(i) << " " << m_qLens.at(i) << std::endl;
			}
		}
};

int main (int argc, char *argv[])
{
	uint32_t /*n=3, f=5,*/ delay = 4;//, maxBytes=100000;
 	uint64_t dataRate = 100 * ONEMBPS;
	std::string queueSize = "100p";
	uint32_t K = 20, qLen = 200;
	double g = 0.0625;
	bool dctcp =false;

    // Step 2
  	CommandLine cmd;
  	cmd.AddValue("K", "K", K);
  	cmd.AddValue("g", "Congestion estimation factor", g);
    cmd.AddValue("q_Len", "Queue Length", qLen);
    cmd.AddValue("dctcp", "Enable/Disable DCTCP", dctcp);
  	cmd.Parse (argc, argv);

  	Time::SetResolution (Time::NS);
    // Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
    // Config::SetDefault("ns3::Ipv4GlobalRouting::EcmpMode", UintegerValue(ecmpMode));


	// Step 3
	Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(500));
	Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(1));
	Config::SetDefault("ns3::TcpSocketBase::EcnMode", EnumValue(ns3::TcpSocketBase::ClassicEcn));
	Config::SetDefault("ns3::RedQueueDisc::MeanPktSize", UintegerValue(500));
	Config::SetDefault("ns3::RedQueueDisc::UseHardDrop", BooleanValue(false));
	Config::SetDefault("ns3::RedQueueDisc::QW", DoubleValue(1));
	Config::SetDefault("ns3::RedQueueDisc::MaxSize", QueueSizeValue(QueueSize(std::to_string(qLen)+"p")));
	Config::SetDefault("ns3::RedQueueDisc::MinTh", DoubleValue(K));
	Config::SetDefault("ns3::RedQueueDisc::MaxTh", DoubleValue(K));
	Config::SetDefault("ns3::RedQueueDisc::UseEcn", BooleanValue(true));
	Config::SetDefault("ns3::FifoQueueDisc::MaxSize", QueueSizeValue(QueueSize(std::to_string(qLen)+"p")));

	// Step 4
	if (dctcp) {
		Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpDctcp::GetTypeId()));
		Config::SetDefault("ns3::TcpDctcp::g", DoubleValue(g));
	}

  	//LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  	//LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);

	// Step 5
	NodeContainer c;
	c.Create (4);
	
	NodeContainer nodes_AC, nodes_BC, nodes_CD;
	nodes_AC = NodeContainer (c.Get (0), c.Get (2));
	nodes_BC = NodeContainer (c.Get (1), c.Get (2));
	nodes_CD = NodeContainer (c.Get (2), c.Get (3));

	InternetStackHelper stack;
	stack.Install (c);
	
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue(DataRate (dataRate)));
	pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delay)));
	
	// Step 6
	pointToPoint.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue("1p"));

	// Step 7
	TrafficControlHelper tch;
	tch.SetRootQueueDisc (dctcp ? "ns3::FifoQueueDisc" : "ns3::RedQueueDisc");

	NetDeviceContainer device_AC, device_BC, device_CD;
	device_AC = pointToPoint.Install (nodes_AC);
	device_BC = pointToPoint.Install (nodes_BC);
	device_CD = pointToPoint.Install (nodes_CD);
	QueueDiscContainer queueDisc_CD = tch.Install(nodes_CD);

	// Step 8
	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interface_AC = address.Assign (device_AC);
	address.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer interface_BC = address.Assign (device_BC);
	address.SetBase ("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer interface_CD = address.Assign (device_CD);

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	// Step 9
	PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 7999));
  	ApplicationContainer sinkApps = packetSinkHelper.Install (c.Get (3));
  	sinkApps.Start (Seconds (0.0));
  	sinkApps.Stop (Seconds (10.0));
	  
	// Step 10
	AddressValue serverAddr (InetSocketAddress(interface_CD.GetAddress(1), 7999));
	BulkSendHelper clientHelper ("ns3::TcpSocketFactory", Address());
	clientHelper.SetAttribute ("Remote", serverAddr);
  	clientHelper.SetAttribute ("SendSize", UintegerValue (500));
  	clientHelper.SetAttribute ("MaxBytes", UintegerValue (0));

	// Step 11
	ApplicationContainer clientApps = clientHelper.Install (c.Get (0));
    clientApps.Add (clientHelper.Install (c.Get(1)));
  	clientApps.Start (Seconds (0.0));
  	clientApps.Stop (Seconds (10.0));

	// Step 12
	QueueMonitor *qm = new QueueMonitor(Time(0)));
	queueDisc_CD.Get(0)->TraceConnectWithoutContext("PacketsInQueue", MakeCallback(&QueueMonitor::QueueChange, qm));

	// Step 13	
	FlowAnalyzer *fa = new FlowAnalyzer(Time(0));
	Ptr<PacketSink> sink = StaticCast<PacketSink> (sinkApps.Get(0));
	sink->TraceConnectWithoutContext("Rx", MakeCallback(&FlowAnalyzer::RecvPkt, fa));

	Simulator::Run ();
	Simulator::Destroy ();

	// Step 15
	// QueueDisc::Stats st;
	std::string filename = "Hw4/queue-" + (dctcp ? "dctcp-" : "tcp-") + std::to_string(K) + "-" + std::to_string(g) + "-" + std::to_string(qLen) + ".txt";
	qm->SaveQueueLen(filename);

	// Step 14
	// st = qd_2_3[i].Get(0)->GetStats();
	// NS_LOG_UNCOND("Queue " << i << " stats:\n" << st);
	NS_LOG_UNCOND("Avg Queue len " << qm->GetAvgQueueLen());
	NS_LOG_UNCOND("The calculated flow throughput is " << fa->CalcThruPut() << " Mbps");
	// NS_LOG_UNCOND("The calculated flow completion time is " << fa->GetAvgFCT() << " seconds");

	// NS_LOG_UNCOND("Calculated per flow throughput");
	// flowAnalyzer->getPerFlowThruPut();
	// NS_LOG_UNCOND("Calculated per flow completion time");
	// flowAnalyzer->getPerFlowCT();

	delete fa;
	delete qm;
	return 0;
}
