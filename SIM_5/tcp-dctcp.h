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

#ifndef TCP_DCTCP_H
#define TCP_DCTCP_H

#include "ns3/tcp-congestion-ops.h"
namespace ns3 {

/**
 * \ingroup congestionOps
 *
 * \brief An implementation of DCTCP. This model implements all the functionalities mentioned
 * in the DCTCP SIGCOMM paper except dynamic buffer allocation in switches
 */

class TcpDctcp : public virtual TcpNewReno
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Create an unbound tcp socket.
   */
  TcpDctcp ();

  /**
   * \brief Copy constructor
   * \param sock the object to copy
   */
  TcpDctcp (const TcpDctcp& sock);

  /**
   * \brief Destructor
   */
  virtual ~TcpDctcp (void);

  /**
   * \brief Get the name of the TCP flavour
   *
   * \return The name of the TCP
   */
  virtual std::string GetName () const;

  virtual Ptr<TcpCongestionOps> Fork (void);

  /**
   * \brief Reduce congestion window based on DCTCP algorithm (sender)
   *
   * \param tcb internal congestion state
   */
  virtual void ReduceCwnd (Ptr<TcpSocketState> tcb);

  /**
   * \brief Get information from the acked packet (sender)
   *
   * \param tcb internal congestion state
   * \param segmentsAcked count of segments ACKed
   * \param rtt The estimated rtt
   */
  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                          const Time &rtt);
  /**
   * \brief Trigger events/calculations on occurrence of congestion window event
   *
   * \param tcb internal state
   * \param event congestion window event which triggered this function
   */
  virtual void CwndEvent (Ptr<TcpSocketState> tcb,
                          const TcpSocketState::TcpCAEvent_t event);

private:
  /**
   * \brief Process an in-coming CE/non-CE packet (receiver side)
   *
   * \param tcb internal congestion state
   * \param currentCE is the current receiving packet CE-marked or not
   */
  void ProcessCE (Ptr<TcpSocketState> tcb, bool currentCE);




  double m_g;                           //!< Congestion estimation factor
};

} // namespace ns3

#endif /* TCP_DCTCP_H */

