#ifndef NDN_HELLO_PROTOCOL_HPP
#define NDN_HELLO_PROTOCOL_HPP

#include <boost/cstdint.hpp>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ndn-conf-parameter.hpp"

namespace ns3 {
namespace ndn {

class HelloProtocol
{
public:
  HelloProtocol(Simulator::Scheduler& scheduler)
    : m_scheduler(scheduler)
    , m_adjLsaBuildInterval(static_cast<uint32_t>(ADJ_LSA_BUILD_INTERVAL_DEFAULT))
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
    m_adjLsaBuildInterval = ndn::time::seconds(interval);
  }

  const ndn::time::seconds&
  getAdjLsaBuildInterval() const
  {
    return m_adjLsaBuildInterval;
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
  Simulator::Scheduler& m_scheduler;
  static const std::string INFO_COMPONENT;
  static const std::string NLSR_COMPONENT;
  ndn::time::seconds m_adjLsaBuildInterval;
  ConfParameter m_conf;
};

} //namespace ndn
} //namespace ns3
#endif // NDN_HELLO_PROTOCOL_HPP
