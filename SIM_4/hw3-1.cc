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

NS_LOG_COMPONENT_DEFINE ("ECE592HW3");

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

int main() {
	// Just to pass complilation
}