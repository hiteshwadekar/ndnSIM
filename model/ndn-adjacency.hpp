#ifndef NDN_ADJACENT_HPP
#define NDN_ADJACENT_HPP

#include <string>
#include <cmath>
#include <boost/cstdint.hpp>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ndn-conf-parameter.hpp"

namespace ns3 {
namespace ndn {

class Adjacent
{

public:
  enum Status
  {
    STATUS_UNKNOWN = -1,
    STATUS_INACTIVE = 0,
    STATUS_ACTIVE = 1
  };

  Adjacent();

  Adjacent(const Name& an);

  Adjacent(Ptr<Node> connecteNode, const Name& an, const std::string& cfu,  double lc,
           Status s, uint32_t iton, uint64_t faceId);

  const Name&
  getName() const
  {
    return m_name;
  }

  void
  setName(const Name& an)
  {
    m_name = an;
  }

  void
  setConnectedNode(Ptr<Node> connectedNode)
  {
	  m_connectedNode=connectedNode;
  }

  const std::string&
  getConnectingFaceUri() const
  {
    return m_connectingFaceUri;
  }

  void
  setConnectingFaceUri(const std::string& cfu)
  {
    m_connectingFaceUri = cfu;
  }

  uint64_t
  getLinkCost() const
  {
    uint64_t linkCost = static_cast<uint64_t>(ceil(m_linkCost));
    return linkCost;
  }

  void
  setLinkCost(double lc)
  {
    m_linkCost = lc;
  }

  Status
  getStatus() const
  {
    return m_status;
  }

  void
  setStatus(Status s)
  {
	  if ((m_status - s) != 0)
	  {
		  m_isStatusChanged=true;
	  }
	  m_status = s;
  }


  uint32_t
  getInterestTimedOutNo() const
  {
    return m_interestTimedOutNo;
  }

  void
  setInterestTimedOutNo(uint32_t iton)
  {
    m_interestTimedOutNo = iton;
  }

  uint32_t
  getInterestSentNo() const
  {
    return m_interestSentCounter;
  }

  void
  setInterestSentNo(uint32_t iton)
  {
	  m_interestSentCounter = iton;
  }

  uint32_t
  getDataRcvNo() const
  {
    return m_DataRcvCounter;
  }

  void
  setDataRcvNo(uint32_t iton)
  {
	  m_DataRcvCounter = iton;
  }

  void
  setFaceId(uint64_t faceId)
  {
    m_faceId = faceId;
  }

  uint64_t
  getFaceId()
  {
    return m_faceId;
  }

  Ptr<Node>
  getConnectedNode()
  {
	  return m_connectedNode;
  }

  bool
  operator==(const Adjacent& adjacent) const;


  inline bool
  compare(const Name& adjacencyName)
  {
    return m_name == adjacencyName;
  }

  inline bool
  compareFaceId(uint64_t faceId)
  {
    return m_faceId == faceId;
  }

  inline bool
  compareFaceUri(std::string& faceUri)
  {
    return m_connectingFaceUri == faceUri;
  }

  inline bool
  getChangedStatus()
  {
	  return m_isStatusChanged;
  }

  void
  writeLog();

public:
  static const float DEFAULT_LINK_COST;

private:
  Ptr<Node> m_connectedNode;
  Name m_name;
  std::string m_connectingFaceUri;
  double m_linkCost;
  Status m_status;
  uint32_t m_interestTimedOutNo;
  uint32_t m_interestSentCounter;
  uint32_t m_DataRcvCounter;
  uint64_t m_faceId;
  ConfParameter m_conf;
  bool m_isStatusChanged;

};
}
}
#endif //NDN_ADJACENT_HPP
