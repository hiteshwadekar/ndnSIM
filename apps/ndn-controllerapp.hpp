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
#include "ns3/names.h"

#include <set>
#include <map>



#include "helper/ndn-controller-string-parser.hpp"
#include "helper/ndn-controller-node-container.hpp"
#include "model/ndn-controller-router.hpp"
#include "model/ndn-yanGraph.hpp"

#include "model/ndn-adjacency-list.hpp"
#include "model/ndn-adjacency.hpp"
#include "model/ndn-conf-parameter.hpp"
#include "core/scheduler.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

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

  virtual void
  OnTimeout(uint32_t sequenceNumber);

  std::string extractNodeName(std::string strPacketName, int n);
  //std::string extractNodeRequestType(std::string strPrefixName);
  std::string extractNodeRequestType(std::string strPrefixName, int index);
  void sendInterestPacket(std::string strPrefix);
  void sendPathDataPacket(shared_ptr<const Interest> interest);
  void extractNodeLinkInfo(std::string strNodeLinkInfo);
  std::string getTheCalculationPath(std::string strForNode);
  void AddIncidency(Ptr<ControllerRouter> node, std::vector<string> fields);
  void AddPrefix(Ptr<ControllerRouter> node, std::vector<string> fields);
  Ptr<ControllerRouter> IsNodePresent(std::string strNodeName);
  bool IsNodeActive(Ptr<ControllerRouter> node);
  void CalculateRoutes();
  void CalculateKPathYanAlgorithm(int kpath);
  std::string GetLocalLinkInfo();
  std::string getPrefix(Ptr<Node> NodeObj);
  void AddControllerNodeInfo(Ptr<ControllerRouter> ControllerRouterNode);
  void StartSendingPathToNode();
  std::string getNodePathData(Ptr<ControllerRouter> dstNode);
  void LinkInitalization(Ptr<ControllerRouter>,shared_ptr<Face>,Ptr<ControllerRouter>);
  void initCalculationKPath();
  void Initialize();
  std::shared_ptr<nfd::Face> GetFaceId(Ptr<ControllerRouter> srcNode, Ptr<ControllerRouter> dstNode);
  void CalculateRoutesSinglePath();


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

  uint32_t m_seq;      ///< @brief currently requested sequence number
  uint32_t m_seqMax;   ///< @brief maximum number of sequence number
  EventId m_sendEvent; ///< @brief EventId of pending "send packet" event
  Time m_retxTimer;    ///< @brief Currently estimated retransmission timer
  EventId m_retxEvent; ///< @brief Event to check whether or not retransmission should be performed

  Name m_keyLocator;
  double m_frequency; // Frequency of interest packets (in hertz)
  Time               m_offTime;             ///< \brief Time interval between packets
  Name     m_interestName;        ///< \brief NDN Name of the Interest (use Name)
  Time               m_interestLifeTime;    ///< \brief LifeTime for interest packet
  ControllerNodeContainer m_controller_node_container;
  Graph my_graph;


  struct RetxSeqsContainer : public std::set<uint32_t> {
    };

  RetxSeqsContainer m_retxSeqs; ///< \brief ordered set of sequence numbers to be retransmitted

  // Hello packets implementation
  static const std::string INFO_COMPONENT;
  static const std::string HELLO_COMPONENT;

  static int counter;

  AdjacencyList m_gb_adList;
  AdjacencyList m_lc_adList;

  time::seconds m_adjControllerBuildInterval;
  ConfParameter m_conf;

  void initialize();
  AdjacencyList CollectLinks();
  void scheduleHelloPacketEvent(uint32_t seconds);
  void sendScheduledHelloInterest(uint32_t seconds);
  void expressInterest(const Name& interestName, uint32_t seconds);
  void SendHelloDataPacket(shared_ptr<const Interest> interest);
  bool m_firstTimeHello;
  std::shared_ptr<ns3::EventId> m_helloEvent;
  std::shared_ptr<ns3::EventId> m_checkEvent;
  std::shared_ptr<ns3::EventId> m_failEvent;
  void schedulecheckLinkEvent(uint32_t seconds);
  void VerifyLinks(uint32_t seconds);
  std::string SendUpdateToController();
  std::stringstream m_strUpdateToController;
  void ControllerSync(std::stringstream& strUpdateToController);
  void SendUpdateDataPacketToController(shared_ptr<const Interest> interest);
  void UpdateIncidency(Ptr<ControllerRouter> node, std::vector<string> fields);
  void scheduleFailEvent(uint32_t seconds);
  void SchedulerHandlingFailureCalc(uint32_t seconds);
};


} // namespace ndn
} // namespace ns3

#endif // NDN_CONTROLLER_H
