/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 University of California, Los Angeles
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
 * Author: Ilya Moiseenko <iliamo@cs.ucla.edu>
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef NDN_CONTROLLER_H
#define NDN_CONTROLLER_H

//#include "ndn-app.hpp"

#include "ndn-app.hpp"
//#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ns3/ptr.h"
#include "ns3/random-variable.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"

#include <set>
#include <map>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>




namespace ns3 {
namespace ndn {
/**
 * @ingroup ndn-apps
 * @brief A simple Interest-sink applia simple Interest-sink application
 *
 * A simple Interest-sink applia simple Interest-sink application,
 * which replying every incoming Interest with Data packet with a specified
 * size and name same as in Interest.cation, which replying every incoming Interest
 * with Data packet with a specified size and name same as in Interest.
 */
class ControllerApp : public App
{
public:
  static TypeId
  GetTypeId (void);

  ControllerApp ();
  virtual ~ControllerApp () {};

  // inherited from NdnApp
  virtual void
  OnInterest (shared_ptr<const Interest> interest);

  virtual void
  OnNack (shared_ptr<const Interest> interest);

  virtual void
  OnData (shared_ptr<const Data> contentObject);

  std::string extractNodeName(std::string strPacketName, int n);
  std::string extractNodeRequestType(std::string strPrefixName);
  void sendInterestPacket(std::string strPrefix);
  void sendDataPacket(shared_ptr<const Interest> interest);
  void extractNodeLinkInfo(std::string strNodeLinkInfo);

protected:
  // inherited from Application base class.
  virtual void
  StartApplication ();    // Called at time specified by Start

  virtual void
  StopApplication ();     // Called at time specified by Stop

protected:
  Name m_prefix;
  Name m_postfix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;
  //Ptr<const Interest> con_pkt_interest;
  UniformVariable m_rand; ///< @brief nonce generator
  uint32_t m_signature;


  Name m_keyLocator;
  double m_frequency; // Frequency of interest packets (in hertz)
  Time               m_offTime;             ///< \brief Time interval between packets
  Name     m_interestName;        ///< \brief NDN Name of the Interest (use Name)
  Time               m_interestLifeTime;    ///< \brief LifeTime for interest packet

};

} // namespace ndn
} // namespace ns3

#endif // NDN_CONTROLLER_H
