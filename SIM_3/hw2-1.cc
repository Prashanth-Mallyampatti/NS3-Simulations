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
	for(int i = temp*me; i < (temp + 1)*me; i++)
	{
		for(int j = temp*ma; j < (temp + 1)*ma; j++)
		{
			e2a[ptr] = NodeContainer (cme.Get(i), cma.Get(j));
			ptr = ptr + 1;
		}
	}
	temp = temp + 1;
	}
	ptr = 0;
	for(int i = 0; i < (ma * k); i++)
	{
		for(int j = 0; j < ma; j++)
		{
			a2c[ptr] = NodeContainer (cma.Get(i), cmc.Get(ptr2));
			ptr = ptr + 1;
			ptr2 = ptr2 + 1;
		}
		if(ptr2 == 16)
			ptr2 =0;
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

	Ipv4AddressGenerator::Init (Ipv4Address ("10.0.0.0"), Ipv4Mask ("/24"));
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

}

