#include "ndn-hello-protocol.hpp"
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/util/scheduler.hpp>

namespace ns3 {
namespace ndn {

#if 0

const std::string HelloProtocol::INFO_COMPONENT = "INFO";
const std::string HelloProtocol::HELLO_COMPONENT = "HELLO";

void
HelloProtocol::expressInterest(const Name& interestName, uint32_t seconds)
{
  cout<< "Expressing Interest :" << interestName << endl;
  Interest i(interestName);
  i.setInterestLifetime(ndn::time::seconds(seconds));
  i.setMustBeFresh(true);
  m_face.expressInterest(i,ndn::bind(&HelloProtocol::onContent,this,_1, _2),ndn::bind(&HelloProtocol::processInterestTimedOut,
                                                 this, _1));


}

void
HelloProtocol::sendScheduledInterest(uint32_t seconds)
{
  std::list<Adjacent> adjList = m_nlsr.getAdjacencyList().getAdjList();
  for (std::list<Adjacent>::iterator it = adjList.begin(); it != adjList.end();
       ++it) {
    if((*it).getFaceId() != 0) {
      /* interest name: /<neighbor>/NLSR/INFO/<router> */
      Name interestName = (*it).getName() ;
      interestName.append(HELLO_COMPONENT);
      interestName.append(INFO_COMPONENT);
      interestName.append(m_conf.getRouterPrefix().wireEncode());
      expressInterest(interestName,m_conf.getInterestResendTime());
    }
    else {
      registerPrefixes((*it).getName(), (*it).getConnectingFaceUri(),
                       (*it).getLinkCost(), ndn::time::milliseconds::max());
    }
  }
  scheduleInterest(m_conf.getInfoInterestInterval());
}

void
HelloProtocol::scheduleInterest(uint32_t seconds)
{
	Simulator::Schedule(Seconds(seconds), ndn::bind(&HelloProtocol::sendScheduledInterest, this, seconds));
  //m_scheduler.scheduleEvent(ndn::time::seconds(seconds),
  //                          ndn::bind(&HelloProtocol::sendScheduledInterest, this, seconds));
}

void
HelloProtocol::processInterest(const ndn::Name& name,
                               const ndn::Interest& interest)
{
  /* interest name: /<neighbor>/NLSR/INFO/<router> */
  const ndn::Name interestName = interest.getName();
  cout <<"Interest Received for Name: " << interestName << endl;
  if (interestName.get(-2).toUri() != INFO_COMPONENT) {
    return;
  }
  ndn::Name neighbor;
  neighbor.wireDecode(interestName.get(-1).blockFromValue());
  cout<<"Neighbor: " << neighbor << endl;
  if (m_adList.isNeighbor(neighbor)) {
    ndn::shared_ptr<ndn::Data> data = ndn::make_shared<ndn::Data>();
    data->setName(ndn::Name(interest.getName()).appendVersion());
    data->setFreshnessPeriod(ndn::time::seconds(10)); // 10 sec
    data->setContent(reinterpret_cast<const uint8_t*>(INFO_COMPONENT.c_str()),
                    INFO_COMPONENT.size());
   // m_nlsr.getKeyChain().sign(*data, m_nlsr.getDefaultCertName());
    cout << "Sending out data for name: " << interest.getName() << endl;
    m_face->put(*data);
    Adjacent *adjacent = m_adList.findAdjacent(neighbor);
    if (adjacent->getStatus() == Adjacent::STATUS_INACTIVE) {
      if(adjacent->getFaceId() != 0){
        /* interest name: /<neighbor>/NLSR/INFO/<router> */
        ndn::Name interestName(neighbor);
        interestName.append(HELLO_COMPONENT);
        interestName.append(INFO_COMPONENT);
        interestName.append(m_conf.getRouterPrefix().wireEncode());
        expressInterest(interestName,
                        m_conf.getInterestResendTime());
      }
      else {
        registerPrefixes(adjacent->getName(), adjacent->getConnectingFaceUri(),
                         adjacent->getLinkCost(), ndn::time::milliseconds::max());
      }
    }
  }
}

void
HelloProtocol::processInterestTimedOut(const ndn::Interest& interest)
{
  /* interest name: /<neighbor>/NLSR/INFO/<router> */
  const ndn::Name interestName(interest.getName());
  cout << "Interest timed out for Name: " << interestName <<endl;
  if (interestName.get(-2).toUri() != INFO_COMPONENT) {
    return;
  }
  ndn::Name neighbor = interestName.getPrefix(-3);
  cout << "Neighbor: "<< neighbor << endl;
  m_adList.incrementTimedOutInterestCount(neighbor);

  Adjacent::Status status = m_adList.getStatusOfNeighbor(neighbor);

  uint32_t infoIntTimedOutCount =
		  m_adList.getTimedOutInterestCount(neighbor);
  cout << "Status: " << status<<endl;
  cout << "Info Interest Timed out: " << infoIntTimedOutCount<<endl;
  if ((infoIntTimedOutCount < m_conf.getInterestRetryNumber())) {
    /* interest name: /<neighbor>/NLSR/INFO/<router> */
    ndn::Name interestName(neighbor);
    interestName.append(HELLO_COMPONENT);
    interestName.append(INFO_COMPONENT);
    interestName.append(m_conf.getRouterPrefix().wireEncode());
    expressInterest(interestName,
                    m_conf.getInterestResendTime());
  }
  else if ((status == Adjacent::STATUS_ACTIVE) &&
           (infoIntTimedOutCount == m_conf.getInterestRetryNumber())) {
	/*
	m_adList.setStatusOfNeighbor(neighbor, Adjacent::STATUS_INACTIVE);
    m_nlsr.incrementAdjBuildCount();
    if (m_nlsr.getIsBuildAdjLsaSheduled() == false) {
      _LOG_DEBUG("Scheduling scheduledAdjLsaBuild");
      m_nlsr.setIsBuildAdjLsaSheduled(true);
      // event here
      m_scheduler.scheduleEvent(m_adjControllerBuildInterval,
                                ndn::bind(&Lsdb::scheduledAdjLsaBuild, &m_nlsr.getLsdb()));
    */
    }
  }
}

void
HelloProtocol::onContent(const ndn::Interest& interest, const ndn::Data& data)
{
  cout << "Received data for INFO(name): " << data.getName() << endl;

  /*
  if (data.getSignature().hasKeyLocator()) {
    if (data.getSignature().getKeyLocator().getType() == ndn::KeyLocator::KeyLocator_Name) {
      _LOG_DEBUG("Data signed with: " << data.getSignature().getKeyLocator().getName());
    }
  }
  m_nlsr.getValidator().validate(data,
                                 ndn::bind(&HelloProtocol::onContentValidated, this, _1),
                                 ndn::bind(&HelloProtocol::onContentValidationFailed,
  */                                         this, _1, _2));
}

void
HelloProtocol::onContentValidated(const ndn::shared_ptr<const ndn::Data>& data)
{
  /* data name: /<neighbor>/NLSR/INFO/<router>/<version> */
  /*

  ndn::Name dataName = data->getName();
  _LOG_DEBUG("Data validation successful for INFO(name): " << dataName);
  if (dataName.get(-3).toUri() == INFO_COMPONENT) {
    ndn::Name neighbor = dataName.getPrefix(-4);

    Adjacent::Status oldStatus = m_nlsr.getAdjacencyList().getStatusOfNeighbor(neighbor);
    m_nlsr.getAdjacencyList().setStatusOfNeighbor(neighbor, Adjacent::STATUS_ACTIVE);
    m_nlsr.getAdjacencyList().setTimedOutInterestCount(neighbor, 0);
    Adjacent::Status newStatus = m_nlsr.getAdjacencyList().getStatusOfNeighbor(neighbor);

    _LOG_DEBUG("Neighbor : " << neighbor);
    _LOG_DEBUG("Old Status: " << oldStatus << " New Status: " << newStatus);
    // change in Adjacency list
    if ((oldStatus - newStatus) != 0) {
      m_nlsr.incrementAdjBuildCount();
      // Need to schedule event for Adjacency LSA building
      if (m_nlsr.getIsBuildAdjLsaSheduled() == false) {
        _LOG_DEBUG("Scheduling scheduledAdjLsaBuild");
        m_nlsr.setIsBuildAdjLsaSheduled(true);
        // event here
        m_scheduler.scheduleEvent(m_adjControllerBuildInterval,
                                  ndn::bind(&Lsdb::scheduledAdjLsaBuild, &m_nlsr.getLsdb()));
      }
    }
  }
  */
}

void
HelloProtocol::onContentValidationFailed(const ndn::shared_ptr<const ndn::Data>& data,
                                         const std::string& msg)
{
  cout << "Validation Error: " << msg << endl;
}

void
HelloProtocol::registerPrefixes(const ndn::Name& adjName, const std::string& faceUri,
                               double linkCost, const ndn::time::milliseconds& timeout)
{
 /*
 m_nlsr.getFib().registerPrefix(adjName, faceUri, linkCost, timeout,
                                 ndn::nfd::ROUTE_FLAG_CAPTURE, 0,
                                 ndn::bind(&HelloProtocol::onRegistrationSuccess,
                                           this, _1, adjName,timeout),
                                 ndn::bind(&HelloProtocol::onRegistrationFailure,
  */                                         this, _1, _2, adjName));
}

void
HelloProtocol::onRegistrationSuccess(const ndn::nfd::ControlParameters& commandSuccessResult,
                                     const ndn::Name& neighbor,const ndn::time::milliseconds& timeout)
{
  /*
  Adjacent *adjacent = m_adList.findAdjacent(neighbor);
  if (adjacent != 0) {
    adjacent->setFaceId(commandSuccessResult.getFaceId());
    ndn::Name broadcastKeyPrefix = DEFAULT_BROADCAST_PREFIX;
    broadcastKeyPrefix.append("KEYS");
    std::string faceUri = adjacent->getConnectingFaceUri();
    double linkCost = adjacent->getLinkCost();
    m_nlsr.getFib().registerPrefix(m_conf.getChronosyncPrefix(),
                                 faceUri, linkCost, timeout,
                                 ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
    m_nlsr.getFib().registerPrefix(m_conf.getLsaPrefix(),
                                 faceUri, linkCost, timeout,
                                 ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
    m_nlsr.getFib().registerPrefix(broadcastKeyPrefix,
                                 faceUri, linkCost, timeout,
                                 ndn::nfd::ROUTE_FLAG_CAPTURE, 0);
    m_nlsr.setStrategies();

    /* interest name: /<neighbor>/NLSR/INFO/<router> */
    /*
	ndn::Name interestName(neighbor);
    interestName.append(HELLO_COMPONENT);
    interestName.append(INFO_COMPONENT);
    interestName.append(m_conf.getRouterPrefix().wireEncode());
    expressInterest(interestName,
                    m_conf.getInterestResendTime());
   */
  }
}

void
HelloProtocol::onRegistrationFailure(uint32_t code, const std::string& error,
                                     const Name& name)
{

  //cout << error << " (code: " << code << ")" << endl;
  /*
  * If NLSR can not create face for given faceUri then it will treat this
  * failure as one INFO interest timed out. So that NLSR can move on with
  * building Adj Lsa and calculate routing table. NLSR does not build Adj
  * Lsa unless all the neighbors are ACTIVE or DEAD. For considering the
  * missconfigured(link) neighbour dead this is required.
  */
  /*
  Adjacent *adjacent = m_nlsr.getAdjacencyList().findAdjacent(name);
  if (adjacent != 0) {
    adjacent->setInterestTimedOutNo(adjacent->getInterestTimedOutNo() + 1);
    Adjacent::Status status = adjacent->getStatus();
    uint32_t infoIntTimedOutCount = adjacent->getInterestTimedOutNo();

    if (infoIntTimedOutCount == m_conf.getInterestRetryNumber()) {
      if (status == Adjacent::STATUS_ACTIVE) {
        adjacent->setStatus(Adjacent::STATUS_INACTIVE);
      }
      m_nlsr.incrementAdjBuildCount();
      if (m_nlsr.getIsBuildAdjLsaSheduled() == false) {
        _LOG_DEBUG("Scheduling scheduledAdjLsaBuild");
        m_nlsr.setIsBuildAdjLsaSheduled(true);
        // event here
        m_scheduler.scheduleEvent(m_adjControllerBuildInterval,
                                  ndn::bind(&Lsdb::scheduledAdjLsaBuild, &m_nlsr.getLsdb()));
      }
    }
  }
  */
}

#endif

} //namespace ndn
} // namespace ns3
