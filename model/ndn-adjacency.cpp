#include <iostream>
#include <string>
#include <cmath>
#include <limits>

#include "ns3/log.h"
#include "ndn-adjacency.hpp"

namespace ns3 {
namespace ndn {

using namespace std;

const float Adjacent::DEFAULT_LINK_COST = 10.0;

Adjacent::Adjacent()
    : m_connectedNode(NULL)
    , m_name()
    , m_connectingFaceUri()
    , m_linkCost(DEFAULT_LINK_COST)
    , m_status(STATUS_INACTIVE)
    , m_interestTimedOutNo(0)
    , m_faceId(0)
{
}

Adjacent::Adjacent(const Name& an)
    : m_name(an)
	, m_connectedNode(NULL)
    , m_connectingFaceUri()
    , m_linkCost(DEFAULT_LINK_COST)
    , m_status(STATUS_INACTIVE)
    , m_interestTimedOutNo(0)
    , m_faceId(0)
  {
  }

Adjacent::Adjacent(Ptr<Node> connectedNode, const Name& an, const std::string& cfu,  double lc,
                   Status s, uint32_t iton, uint64_t faceId)
    :m_connectedNode(connectedNode)
    , m_name(an)
    , m_connectingFaceUri(cfu)
    , m_linkCost(lc)
    , m_status(s)
    , m_interestTimedOutNo(iton)
    , m_faceId(faceId)
  {

  }

bool
Adjacent::operator==(const Adjacent& adjacent) const
{

  /*return (m_name == adjacent.getName()) &&
         (std::abs(m_linkCost - adjacent.getLinkCost()) <
          std::numeric_limits<double>::epsilon()) && (m_faceId == adjacent.getFaceId()) && (m_status == adjacent.getStatus());*/
  return (m_name == adjacent.getName()) && (m_status == adjacent.getStatus()) && (m_connectingFaceUri == adjacent.getConnectingFaceUri()) &&
          (std::abs(m_linkCost - adjacent.getLinkCost()) <
           std::numeric_limits<double>::epsilon());

}

void
Adjacent::writeLog()
{
	cout<<"Adjacent : " << m_name <<endl;
	cout<<"FaceId: " << m_faceId <<endl;
	cout<<"Connecting FaceUri: " << m_connectingFaceUri<<endl;
	cout<<"Link Cost: " << m_linkCost <<endl;
	cout<<"Status: " << m_status <<endl;
	cout<<"Interest Timed out: " << m_interestTimedOutNo<<endl;
	cout<<"No of interest send: " << m_interestSentCounter<<endl;
	cout<<"No of data received: " << m_DataRcvCounter<<endl;

}

}//namespace ndn
} //namespace ns3
