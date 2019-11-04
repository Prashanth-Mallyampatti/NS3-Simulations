/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
 *
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
 *
 * Author: Shravya K.S. <shravya.ks0@gmail.com>
 *
 */

#include "tcp-dctcp.h"
#include "ns3/log.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "ns3/node.h"
#include "math.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/sequence-number.h"
#include "ns3/double.h"
#include "ns3/nstime.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpDctcp");

NS_OBJECT_ENSURE_REGISTERED (TcpDctcp);

TypeId TcpDctcp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpDctcp")
    .SetParent<TcpNewReno> ()
    .AddConstructor<TcpDctcp> ()
    .SetGroupName ("Internet")
    .AddAttribute ("g",
                   "Value of g for updating DCTCP.Alpha",
                   DoubleValue (0.0625),
                   MakeDoubleAccessor (&TcpDctcp::m_g),
                   MakeDoubleChecker<double> (0))
  ;
  return tid;
}

std::string TcpDctcp::GetName () const
{
  return "TcpDctcp";
}

TcpDctcp::TcpDctcp ()
  : TcpNewReno ()
{
  NS_LOG_FUNCTION (this);
}

TcpDctcp::TcpDctcp (const TcpDctcp& sock)
  : TcpNewReno (sock)
{
  NS_LOG_FUNCTION (this);
  m_g = sock.m_g;
}

TcpDctcp::~TcpDctcp (void)
{
  NS_LOG_FUNCTION (this);
}

Ptr<TcpCongestionOps> TcpDctcp::Fork (void)
{
  NS_LOG_FUNCTION (this);
  return CopyObject<TcpDctcp> (this);
}

void
TcpDctcp::ReduceCwnd (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);

  /***************************************************/
  //** DCTCP: Fill your code here

  // Rather than always halving the congestion window as described in
  // [RFC3168], the sender SHOULD update cwnd as follows:
  //   cwnd = cwnd * (1 - DCTCP.Alpha / 2)

  uint32_t cWnd = (int) tcb->m_cWnd * (1 - tcb->m_alpha /2.0);
  tcb->m_cWnd = cWnd;


  /***************************************************/
}

void
TcpDctcp::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked, const Time &rtt)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked << rtt);

  /***************************************************/
  //** DCTCP: Fill your code here

  // 1.  Compute the bytes acknowledged (TCP Selective Acknowledgment
  //      (SACK) options [RFC2018] are ignored for this computation):
  //         BytesAcked = SEG.ACK - SND.UNA
  uint32_t bytesAcked = segmentsAcked - (tcb->m_lastAckedSeq+1);

  //  2.  Update the bytes sent:
  //         DCTCP.BytesAcked += BytesAcked
  tcb->m_bytesAcked += bytesAcked;

  //  3.  If the ECE flag is set, update the bytes marked:
  //         DCTCP.BytesMarked += BytesAcked
  tcb->m_bytesMarked += bytesAcked;

  //  4.  If the acknowledgment number is less than or equal to
  //      DCTCP.WindowEnd, stop processing.  Otherwise, the end of the
  //      observation window has been reached, so proceed to update the
  //      congestion estimate as follows:
  if (tcb->m_lastAckedSeq > tcb->m_nextTxSequence-1) {

    //  5.  Compute the congestion level for the current observation window:
    //         M = DCTCP.BytesMarked / DCTCP.BytesAcked
    uint32_t m = tcb->m_bytesMarked / tcb->m_bytesAcked;

    //  6.  Update the congestion estimate:
    //         DCTCP.Alpha = DCTCP.Alpha * (1 - g) + g * M
    tcb->m_alpha = tcb->m_alpha * (1-m_g) + m_g * m;

    //  7.  Determine the end of the next observation window:
    //         DCTCP.WindowEnd = SND.NXT
    tcb->m_windowEnd = m_nextTxSequence;

    //  8.  Reset the byte counters:
    //         DCTCP.BytesAcked = DCTCP.BytesMarked = 0
    tcb->m_bytesAcked = 0;
    tcb->m_bytesMarked = 0;
  }
  /***************************************************/
}

void
TcpDctcp::ProcessCE (Ptr<TcpSocketState> tcb, bool currentCE)
{
  NS_LOG_FUNCTION (this << tcb);

  /***************************************************/
  //** DCTCP: Fill your code here
  
  // Set ECN_CE_RCVD if CE bit was set
  tcb->m_ecnState = ECN_CE_RCVD;

  /***************************************************/
}

void
TcpDctcp::CwndEvent (Ptr<TcpSocketState> tcb,
                     const TcpSocketState::TcpCAEvent_t event)
{
  NS_LOG_FUNCTION (this << tcb << event);
  switch (event)
    {
    case TcpSocketState::CA_EVENT_ECN_IS_CE:
      ProcessCE (tcb, true);
      break;
    case TcpSocketState::CA_EVENT_ECN_NO_CE:
      ProcessCE (tcb, false);
      break;
    default:
      /* Don't care for the rest. */
      break;
    }

}
}