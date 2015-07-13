#ifndef NDN_CONSUMER_CHANGE_H
#define NDN_CONSUMER_CHANGE_H

//#include "ns3/ndnSIM/model/ndn-common.hpp"

#include "ndn-app.hpp"

#include "ns3/random-variable.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/utils/ndn-rtt-estimator.hpp"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.hpp"

#include <set>
#include <map>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include "model/ndn-net-device-face.hpp"
#include "ns3/random-variable.h"
#include "ns3/ptr.h"
#include "model/ndn-adjacency-list.hpp"
#include "model/ndn-adjacency.hpp"
#include "model/ndn-conf-parameter.hpp"
#include "core/scheduler.hpp"

namespace ns3 {
namespace ndn {

/**
 * @ingroup ndn-apps
 * \brief NDN application for sending out Interest packets
 */
class ConsumerChange : public App {
public:
  static TypeId
  GetTypeId();

  /**
   * \brief Default constructor
   * Sets up randomizer function and packet sequence number
   */
  ConsumerChange();

  ~ConsumerChange();

  // From App
  virtual void
  OnInterest (std::shared_ptr<const Interest> interest);

  // From App
  virtual void
  OnData(shared_ptr<const Data> contentObject);

  /**
   * @brief Timeout event
   * @param sequenceNumber time outed sequence number
   */
  virtual void
  OnTimeout(uint32_t sequenceNumber);

  /**
   * @brief Actually send packet
   */
  void
  SendPacket();

  /**
   * @brief An event that is fired just before an Interest packet is actually send out (send is
   *inevitable)
   *
   * The reason for "before" even is that in certain cases (when it is possible to satisfy from the
   *local cache),
   * the send call will immediately return data, and if "after" even was used, this after would be
   *called after
   * all processing of incoming data, potentially producing unexpected results.
   */
  virtual void
  WillSendOutInterest(uint32_t sequenceNumber);

protected:
  // from App
  virtual void
  StartApplication();

  virtual void
  StopApplication();

  /**
   * \brief Checks if the packet need to be retransmitted becuase of retransmission timer expiration
   */
  void
  CheckRetxTimeout();

  /**
   * \brief Modifies the frequency of checking the retransmission timeouts
   * \param retxTimer Timeout defining how frequent retransmission timeouts should be checked
   */
  void
  SetRetxTimer(Time retxTimer);

  /**
   * \brief Returns the frequency of checking the retransmission timeouts
   * \return Timeout defining how frequent retransmission timeouts should be checked
   */
  Time
  GetRetxTimer() const;

  /**
   * \brief Constructs the Interest packet and sends it using a callback to the underlying NDN
   * protocol
   */
  virtual void
  ScheduleNextPacket();

  /**
   * @brief Set type of frequency randomization
   * @param value Either 'none', 'uniform', or 'exponential'
   */
  void
  SetRandomize(const std::string& value);

  /**
   * @brief Get type of frequency randomization
   * @returns either 'none', 'uniform', or 'exponential'
   */
  std::string
  GetRandomize() const;

protected:
  bool m_firstTime;
  //--------------------------------------------------------------------------------------------
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

  std::string m_ProducerDataTestInterest;        ///< \brief NDN Name of the Interest (use Name)

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
  std::shared_ptr<ns3::EventId> m_boostLinkEvent;
  std::shared_ptr<ns3::EventId> m_scheduleInterestProducer;
  void initialize();
  AdjacencyList CollectLinks();

  void scheduleHelloPacketEvent(uint32_t seconds);
  void schedulecheckLinkEvent(uint32_t seconds);
  void scheduleFailEvent(uint32_t seconds);
  void sendScheduledHelloInterest(uint32_t seconds);
  void scheduleBoostLinkCost(uint32_t seconds);
  void schedulePacketProducer(uint32_t seconds);

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
  void boostLinkCost(uint32_t sequenceNumber);
  //-----------------------------------------------------------------------------------------------


  //UniformVariable m_rand; ///< @brief nonce generator

  uint32_t m_seq;      ///< @brief currently requested sequence number
  uint32_t m_seqMax;   ///< @brief maximum number of sequence number
  EventId m_sendEvent; ///< @brief EventId of pending "send packet" event
  Time m_retxTimer;    ///< @brief Currently estimated retransmission timer
  EventId m_retxEvent; ///< @brief Event to check whether or not retransmission should be performed

  Ptr<RttEstimator> m_rtt; ///< @brief RTT estimator

  //Time m_offTime;          ///< \brief Time interval between packets
  //Name m_interestName;     ///< \brief NDN Name of the Interest (use Name)
  //Time m_interestLifeTime; ///< \brief LifeTime for interest packet

  /// @cond include_hidden
  /**
   * \struct This struct contains sequence numbers of packets to be retransmitted
   */
  struct RetxSeqsContainer : public std::set<uint32_t> {
  };

  RetxSeqsContainer m_retxSeqs; ///< \brief ordered set of sequence numbers to be retransmitted

  /**
   * \struct This struct contains a pair of packet sequence number and its timeout
   */
  struct SeqTimeout {
    SeqTimeout(uint32_t _seq, Time _time)
      : seq(_seq)
      , time(_time)
    {
    }

    uint32_t seq;
    Time time;
  };
  /// @endcond

  /// @cond include_hidden
  class i_seq {
  };
  class i_timestamp {
  };
  /// @endcond

  /// @cond include_hidden
  /**
   * \struct This struct contains a multi-index for the set of SeqTimeout structs
   */
  struct SeqTimeoutsContainer
    : public boost::multi_index::
        multi_index_container<SeqTimeout,
                              boost::multi_index::
                                indexed_by<boost::multi_index::
                                             ordered_unique<boost::multi_index::tag<i_seq>,
                                                            boost::multi_index::
                                                              member<SeqTimeout, uint32_t,
                                                                     &SeqTimeout::seq>>,
                                           boost::multi_index::
                                             ordered_non_unique<boost::multi_index::
                                                                  tag<i_timestamp>,
                                                                boost::multi_index::
                                                                  member<SeqTimeout, Time,
                                                                         &SeqTimeout::time>>>> {
  };

  SeqTimeoutsContainer m_seqTimeouts; ///< \brief multi-index for the set of SeqTimeout structs

  SeqTimeoutsContainer m_seqLastDelay;
  SeqTimeoutsContainer m_seqFullDelay;
  std::map<uint32_t, uint32_t> m_seqRetxCounts;

  TracedCallback<Ptr<App> /* app */, uint32_t /* seqno */, Time /* delay */, int32_t /*hop count*/>
    m_lastRetransmittedInterestDataDelay;
  TracedCallback<Ptr<App> /* app */, uint32_t /* seqno */, Time /* delay */,
                 uint32_t /*retx count*/, int32_t /*hop count*/> m_firstInterestDataDelay;

  /// @endcond
};

} // namespace ndn
} // namespace ns3

#endif
