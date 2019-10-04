#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("HW2");

int main (int argc, char *argv[])
{
	int k = 8;
	int mc = pow((k / 2), 2);
	int me = (k / 2);
	int ma = (k / 2);
	int mh = (pow(k, 3)) / 4;

	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

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

	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

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
        if(i == 0)
		{
			Ipv4Address addr = interface_h2e[i].GetAddress(0);
			std::cout << addr << std::endl;
		}
	}
	for(int i = 0; i < mh; i++)
	{
		subnet_e2a[i] = Ipv4AddressGenerator::NextNetwork (Ipv4Mask("/24"));
		address.SetBase (subnet_e2a[i], "255.255.255.0");
		interface_e2a[i] = address.Assign (dev_e2a[i]);
		if(i == 127)
		{
			Ipv4Address addr = interface_e2a[i].GetAddress(0);
			std::cout << addr << std::endl;
		}
	}	
	for(int i = 0; i < mh; i++)
	{
		subnet_a2c[i] = Ipv4AddressGenerator::NextNetwork (Ipv4Mask("/24"));
		address.SetBase (subnet_a2c[i], "255.255.255.0");
		interface_a2c[i] = address.Assign (dev_a2c[i]);
		if(i == 0)
		{
			Ipv4Address addr = interface_a2c[i].GetAddress(0);
			std::cout << addr <<std::endl;
		}
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	UdpEchoServerHelper last_host (7999);
	ApplicationContainer server_last_host = last_host.Install (h2e[127].Get(0));
	server_last_host.Start (Seconds (1.0));
	
	UdpEchoClientHelper client_first_host (interface_h2e[127].GetAddress (0), 7999);
	client_first_host.SetAttribute ("MaxPackets", UintegerValue (3));
	client_first_host.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	client_first_host.SetAttribute ("PacketSize", UintegerValue (1024));
	ApplicationContainer first_host = client_first_host.Install (h2e[0].Get(0));

	first_host.Start (Seconds (2.0));
	pointToPoint.EnablePcap ("Hw2-1", h2e[0].Get (0)->GetId (), true);
	pointToPoint.EnablePcap ("Hw2-1", h2e[127].Get (0)->GetId (), true);

	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
