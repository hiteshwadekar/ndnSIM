#ifndef NDN_HELLO_PROTOCOL_HPP
#define NDN_HELLO_PROTOCOL_HPP

#include <boost/cstdint.hpp>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ndn-conf-parameter.hpp"
#include "ndn-adjacency-list.hpp"
#include "model/ndn-net-device-face.hpp"
#include "ndn-adjacency.hpp"

namespace ns3 {
namespace ndn {

#if 0

class HelloProtocol
{
public:
  HelloProtocol(AdjacencyList adList, shared_ptr<Face> face)
    : m_adList(adList)
    , m_face(face)
    , m_adjControllerBuildInterval(static_cast<uint32_t>(m_conf.getAdjLsaBuildInterval()))
  {
  }

  void
  scheduleInterest(uint32_t seconds);

  void
  expressInterest(const Name& interestNamePrefix, uint32_t seconds);

  void
  sendScheduledInterest(uint32_t seconds);

  void
  processInterest(const Name& Prefname, const Interest& interest);

  void
  setAdjLsaBuildInterval(uint32_t interval)
  {
    m_adjControllerBuildInterval = ndn::time::seconds(interval);
  }

  const ndn::time::seconds&
  getAdjLsaBuildInterval() const
  {
    return m_adjControllerBuildInterval;
  }

private:
  void
  processInterestTimedOut(const Interest& interest);

  void
  onContent(const Interest& interest, const Data& data);

  void
  onContentValidated(const shared_ptr<const Data>& data);

  void
  onContentValidationFailed(const shared_ptr<const Data>& data,
                            const std::string& msg);

  void
  onRegistrationFailure(uint32_t code, const std::string& error,
                        const Name& name);

  void
  onRegistrationSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                        const Name& neighbor, const ndn::time::milliseconds& timeout);

  void
  registerPrefixes(const Name& adjName, const std::string& faceUri,
                   double linkCost, const ndn::time::milliseconds& timeout);
private:
  //Scheduler m_scheduler;
  static const std::string INFO_COMPONENT;
  static const std::string HELLO_COMPONENT;
  //time::seconds m_adjControllerBuildInterval;
  time::seconds m_adjControllerBuildInterval;
  ConfParameter m_conf;
  shared_ptr<Face> m_face;
  AdjacencyList m_adList;
};
#endif
} //namespace ndn
} //namespace ns3
#endif // NDN_HELLO_PROTOCOL_HPP
