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

#ifndef NDN_CUSTCONSUMER_H
#define NDN_CUSTCOMSUMER_H

//#include "ndn-app.hpp"

#include "model/ndn-net-device-face.hpp"
#include "ndn-app.hpp"
//#include "ns3/ndnSIM/model/ndn-common.hpp"


//#include "ns3/ndn-name.hpp"
//#include "ns3/ndn-data.hpp"

#include "ns3/random-variable.h"
//#include "ns3/ndn-name.hpp"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
//#include "ns3/ndn-rtt-estimator.hpp"

#include <set>
#include <map>


#include "ns3/nstime.h"
#include "ns3/ptr.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "model/ndn-adjacency-list.hpp"
#include "model/ndn-adjacency.hpp"
#include "model/ndn-conf-parameter.hpp"
#include "core/scheduler.hpp"

using namespace std;

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
class CustConsumer : public App
{
public:
  static TypeId
  GetTypeId (void);

  CustConsumer ();
  virtual ~CustConsumer () {};

  // inherited from NdnApp
  virtual void
  OnInterest (std::shared_ptr<const Interest> interest);

  virtual void
  OnNack (std::shared_ptr<const Interest> interest);

  virtual void
  OnData (std::shared_ptr<const Data> contentObject);

  virtual void
  OnTimeout(uint32_t sequenceNumber);

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
  //list<TopologyReader::Link> m_linksList;
  //Ptr<const Interest> con_pkt_interest;
  UniformVariable m_rand; ///< @brief nonce generator
  uint32_t m_signature;

  RandomVariable* m_random;
  std::string m_randomType;



  Name m_keyLocator;
  double m_frequency; // Frequency of interest packets (in hertz)
  Time m_offTime;             ///< \brief Time interval between packets
  Name m_interestName;        ///< \brief NDN Name of the Interest (use Name)
  Time m_interestLifeTime;    ///< \brief LifeTime for interest packet

  void SendInterestPacket(std::string strPrefixToController);
  void updateNodeLinkInfo(std::string strLinkInfo, bool isFirstTime);
  void SendDataPacket(std::shared_ptr<const Interest> interest, bool toController);
  std::string GetLocalLinkInfo();
  std::string extractNodeName(std::string strPacketName);
  std::string extractNodeRequestType(std::string strPrefixName, int index);
  void getOSPFfromNodeName(std::string FromNodeName, std::string ToNodeName);
  std::string getPrefix(Ptr<Node> Node);
  bool IsFIBMetricsUpdatable(std::string strPrefixName, std::shared_ptr<NetDeviceFace> faceId, size_t faceMetrics);

  // Hello packets implementation
  static const std::string INFO_COMPONENT;
  static const std::string HELLO_COMPONENT;

  static int counter;

  AdjacencyList m_gb_adList;
  AdjacencyList m_lc_adList;
  time::seconds m_adjControllerBuildInterval;
  bool m_firstTimeHello;
  ConfParameter m_conf;
  std::shared_ptr<ns3::EventId> m_helloEvent;
  std::shared_ptr<ns3::EventId> m_checkEvent;
  std::shared_ptr<ns3::EventId> m_failEvent;
  void initialize();
  AdjacencyList CollectLinks();
  void scheduleHelloPacketEvent(uint32_t seconds);
  void schedulecheckLinkEvent(uint32_t seconds);
  void scheduleFailEvent(uint32_t seconds);
  void sendScheduledHelloInterest(uint32_t seconds);
  void expressInterest(const Name& interestName, uint32_t seconds);
  void SendHelloDataPacket(shared_ptr<const Interest> interest);
  void VerifyLinks(uint32_t seconds);
  std::string SendUpdateToController();
  std::stringstream m_strUpdateToController;
  std::string m_strUpdateToController1;

  void ControllerSync(std::stringstream& strUpdateToController);
  void SendUpdateDataPacketToController(shared_ptr<const Interest> interest);
  void sendAckDataPacket(std::shared_ptr<const Interest> interest);
  void unregisterPrefix(std::string strLinkInfo);

 };

} // namespace ndn
} // namespace ns3

#endif // NDN_CUSTCONSUMER_H
