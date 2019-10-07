#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <iostream>
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include <memory>
#include <time.h>

using namespace ns3;

#define ONEMBPS 1000000
NS_LOG_COMPONENT_DEFINE ("HW2");

uint32_t nSink = 64, nSource=64;
bool parse_nSink(const std::string val) {
    int value = std::stoi(val);
    if ((value > 0 && value < 65) && (ceil(log2(value)) == floor(log2(value)))) {
        nSink = value;
		// Step 4
		nSource = 128-nSink;
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
            return ((m_totalRx / (m_lastTime.GetSeconds() - m_startTime.GetSeconds())) / double(ONEMBPS));
        }
};

int main (int argc, char *argv[])
{
	int k = 8;
	int mc = pow((k / 2), 2);
	int me = (k / 2);
	int ma = (k / 2);
	int mh = (pow(k, 3)) / 4;
	uint32_t maxBytes = ONEMBPS, i, delay;
    uint64_t dataRate;
	bool ecmp=false;

    // Step 3
    CommandLine cmd;
    cmd.AddValue("ECMP", "Enable ECMP", ecmp);
    cmd.AddValue("nSink", "Num. of Sinks", MakeCallback(&parse_nSink));
    cmd.AddValue("MaxBytes", "Maximum Bytes", maxBytes);
    cmd.Parse (argc, argv);

    dataRate = 5 * ONEMBPS;
    delay = 2;

    Time::SetResolution (Time::NS);

	// Step 5
	Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(ecmp));

	NodeContainer cmh, cme, cma, cmc;
	cmh.Create(mh);
	cme.Create(me*k);
	cma.Create(ma*k);
	cmc.Create(mc);

	NodeContainer h2e[mh], e2a[mh], a2c[mh];
	int ptr = 0, ptr2 = 0;
	for(int i = 0; i < (me * k); i++)
	{
		for(int j = 0; j < me; j++)
		{
			h2e[ptr] = NodeContainer (cmh.Get(ptr), cme.Get(i));
			ptr = ptr + 1;
		}
	}

	ptr = 0;
	int temp = 0;
	while(temp < k)
	{
		for(int i = temp*ma; i < (temp + 1)*ma; i++)
		{
			for(int j = temp*me; j < (temp + 1)*me; j++)
			{
				e2a[ptr] = NodeContainer (cme.Get(j), cma.Get(i));
				ptr = ptr + 1;
			}
		}
		temp = temp + 1;
	}
	
	ptr = 0;
	temp = 0;
	int temp1 = 0;
	while(temp < mc && temp1 < ma)
	{
		for(int i = temp; i < temp + ma; i++)
		{
			ptr2 = temp1;
			for(int j = 0; j < k; j++)
			{
				a2c[ptr] = NodeContainer (cma.Get(ptr2), cmc.Get(i));
				ptr = ptr + 1;
				ptr2 = ptr2 + ma;
			}
		}
		temp = temp + ma;
		temp1 = temp1 + 1;
	}
	
	InternetStackHelper stack;
	stack.Install (cmh);
	stack.Install (cme);
	stack.Install (cma);
	stack.Install (cmc);

	// Step 6
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute("DataRate", DataRateValue (DataRate (dataRate)));
	pointToPoint.SetChannelAttribute("Delay", TimeValue (MilliSeconds (delay)));

	NetDeviceContainer dev_h2e[mh], dev_e2a[mh], dev_a2c[mh];

	Ipv4AddressGenerator::Init (Ipv4Address ("10.1.0.0"), Ipv4Mask ("/24"));
	Ipv4Address subnet_h2e[mh], subnet_e2a[mh], subnet_a2c[mh];
	
	Ipv4AddressHelper address;
	Ipv4InterfaceContainer interface_h2e[mh], interface_e2a[mh], interface_a2c[mh];

	for(int i = 0; i < mh; i++)
	{
		dev_h2e[i] = pointToPoint.Install (h2e[i]);
		dev_e2a[i] = pointToPoint.Install (e2a[i]);
		dev_a2c[i] = pointToPoint.Install (a2c[i]);
		subnet_h2e[i] = Ipv4AddressGenerator::NextNetwork (Ipv4Mask("/24"));	
		address.SetBase (subnet_h2e[i], "255.255.255.0");
		interface_h2e[i] = address.Assign (dev_h2e[i]);
	}
	for(int i = 0; i < mh; i++)
	{
		subnet_e2a[i] = Ipv4AddressGenerator::NextNetwork (Ipv4Mask("/24"));
		address.SetBase (subnet_e2a[i], "255.255.255.0");
		interface_e2a[i] = address.Assign (dev_e2a[i]);
	}	
	for(int i = 0; i < mh; i++)
	{
		subnet_a2c[i] = Ipv4AddressGenerator::NextNetwork (Ipv4Mask("/24"));
		address.SetBase (subnet_a2c[i], "255.255.255.0");
		interface_a2c[i] = address.Assign (dev_a2c[i]);
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	// Step 7 - Sinks
	PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 7999));
	
	ApplicationContainer sinkApps;
	for(i = nSource; i<(nSource+nSink); i++) {
		sinkApps.Add(packetSinkHelper.Install(h2e[i].Get(0)));
	}
	sinkApps.Start(Seconds(0.0));
	sinkApps.Stop(Seconds(15.0));
	
	// Step 7 - Sources
    OnOffHelper clientHelper ("ns3::UdpSocketFactory", Address());
    clientHelper.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    clientHelper.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    clientHelper.SetAttribute ("PacketSize", UintegerValue (1000));
    clientHelper.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
    clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));

	ApplicationContainer sourceApps;
	for(i=0; i<nSource; i++){
		sourceApps.Add(clientHelper.Install(h2e[i].Get(0)));
	}
	sourceApps.Start(Seconds(5.0));
	sourceApps.Stop(Seconds(15.0));

	// Step 8
	// Gathering sink addresses
	AddressValue serverAddr[nSink];
	for(i=0; i<nSink; i++) {
		serverAddr[i].Set((InetSocketAddress(interface_h2e[nSource+i].GetAddress(0), 7999)));
	}

	// Assigning sink addresses to sources
	for(i=0; i<nSource; i++) {
		Ptr<OnOffApplication> src = StaticCast<OnOffApplication> (sourceApps.Get(i));
		src->SetAttribute ("Remote", serverAddr[i%nSink]);
	}

	// Step 9
    ThruPutAnalyzer* tpAnalyzer = new ThruPutAnalyzer(Time("5s"));
	for(i=0; i<nSink; i++) {
    	Ptr<PacketSink> sink = StaticCast<PacketSink> (sinkApps.Get(i));
    	sink->TraceConnectWithoutContext("Rx", MakeCallback(&ThruPutAnalyzer::RecvPkt, tpAnalyzer));
	}
	
 	// Step 11
	pointToPoint.EnablePcap("Hw2-3/All", cmh);
	pointToPoint.EnablePcap("Hw2-3/All", cme);
	
	Simulator::Run ();
	Simulator::Destroy ();

	NS_LOG_UNCOND("The calculated throughput with " << nSource << " senders is " << tpAnalyzer->CalcThruPut() << " Mbps");
	delete tpAnalyzer;
	return 0;
}

