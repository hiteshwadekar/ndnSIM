#include <algorithm>
#include "ndn-adjacency-list.hpp"
#include "ns3/log.h"
#include "ns3/ndnSIM/model/ndn-common.hpp"

namespace ns3 {
namespace ndn {

using namespace std;


AdjacencyList::AdjacencyList()
{
}

AdjacencyList::~AdjacencyList()
{
}

int32_t
AdjacencyList::insert(Adjacent& adjacent)
{
  std::list<Adjacent>::iterator it = find(adjacent.getName());
  if (it != m_adjList.end()) {
    return -1;
  }
  m_adjList.push_back(adjacent);
  return 0;
}

void
AdjacencyList::addAdjacents(AdjacencyList& adl)
{
  for (std::list<Adjacent>::iterator it = adl.getAdjList().begin();
       it != adl.getAdjList().end(); ++it) {
    insert((*it));
  }
}

bool
AdjacencyList::updateAdjacentStatus(const Name& adjName, Adjacent::Status s)
{
  std::list<Adjacent>::iterator it = find(adjName);

  if (it == m_adjList.end()) {
    return false;
  }
  else {
    it->setStatus(s);
    return true;
  }
}

Adjacent
AdjacencyList::getAdjacent(const Name& adjName)
{
  Adjacent adj(adjName);
  std::list<Adjacent>::iterator it = find(adjName);
  if (it != m_adjList.end()) {
    return (*it);
  }
  return adj;
}

static bool
compareAdjacent(const Adjacent& adjacent1, const Adjacent& adjacent2)
{
  return adjacent1.getName() < adjacent2.getName();
}

bool
AdjacencyList::operator==(AdjacencyList& adl)
{
  if (getSize() != adl.getSize()) {
    return false;
  }
  m_adjList.sort(compareAdjacent);
  adl.getAdjList().sort(compareAdjacent);
  uint32_t equalAdjCount = 0;
  std::list<Adjacent>& adjList2 = adl.getAdjList();
  std::list<Adjacent>::iterator it1;
  std::list<Adjacent>::iterator it2;
  for (it1 = m_adjList.begin(), it2 = adjList2.begin();
       it1 != m_adjList.end(); it1++, it2++) {
    if (!((*it1) == (*it2))) {
      break;
    }
    equalAdjCount++;
  }
  return equalAdjCount == getSize();
}

int32_t
AdjacencyList::updateAdjacentLinkCost(const Name& adjName, double lc)
{
  std::list<Adjacent>::iterator it = find(adjName);
  if (it == m_adjList.end()) {
    return -1;
  }
  (*it).setLinkCost(lc);
  return 0;
}

bool
AdjacencyList::isNeighbor(const Name& adjName)
{
  std::list<Adjacent>::iterator it = find(adjName);
  if (it == m_adjList.end())
  {
    return false;
  }
  return true;
}

void
AdjacencyList::incrementTimedOutInterestCount(const Name& neighbor)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it == m_adjList.end()) {
    return ;
  }
  (*it).setInterestTimedOutNo((*it).getInterestTimedOutNo() + 1);
}

void
AdjacencyList::setTimedOutInterestCount(const Name& neighbor,
                                        uint32_t count)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it != m_adjList.end()) {
    (*it).setInterestTimedOutNo(count);
  }
}

int32_t
AdjacencyList::getTimedOutInterestCount(const Name& neighbor)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it == m_adjList.end()) {
    return -1;
  }
  return (*it).getInterestTimedOutNo();
}

Adjacent::Status
AdjacencyList::getStatusOfNeighbor(const Name& neighbor)
{
  std::list<Adjacent>::iterator it = find(neighbor);

  if (it == m_adjList.end()) {
    return Adjacent::STATUS_UNKNOWN;
  }
  else {
    return it->getStatus();
  }
}

void
AdjacencyList::setStatusOfNeighbor(const Name& neighbor, Adjacent::Status status)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it != m_adjList.end()) {
    it->setStatus(status);
  }
}

std::list<Adjacent>&
AdjacencyList::getAdjList()
{
  return m_adjList;
}

bool
AdjacencyList::isAdjLsaBuildable()
{
  uint32_t nbrCount = 0;
  for (std::list<Adjacent>::iterator it = m_adjList.begin(); it != m_adjList.end() ; it++) {

    if (it->getStatus() == Adjacent::STATUS_ACTIVE) {
      nbrCount++;
    }
    else {
    	if ((*it).getInterestTimedOutNo() >=
    			m_conf.getInterestRetryNumber()) {
        nbrCount++;
      }
    }
  }
  if (nbrCount == m_adjList.size()) {
    return true;
  }
  return false;
}

int32_t
AdjacencyList::getNumOfActiveNeighbor()
{
  int32_t actNbrCount = 0;
  for (std::list<Adjacent>::iterator it = m_adjList.begin(); it != m_adjList.end(); it++) {

    if (it->getStatus() == Adjacent::STATUS_ACTIVE) {
      actNbrCount++;
    }
  }
  return actNbrCount;
}


std::list<Adjacent>::iterator
AdjacencyList::find(const Name& adjName)
{
  std::list<Adjacent>::iterator it = std::find_if(m_adjList.begin(),m_adjList.end(),std::bind(&Adjacent::compare,_1,cref(adjName)));
  return it;
}

Adjacent *
AdjacencyList::findAdjacent(const Name& adjName)
{
  std::list<Adjacent>::iterator it = std::find_if(m_adjList.begin(),
                                                  m_adjList.end(),
                                                  std::bind(&Adjacent::compare,_1,cref(adjName)));
  if (it != m_adjList.end()) {
    return &(*it);
  }

  return 0;
}

Adjacent *
AdjacencyList::findAdjacent(uint64_t faceId)
{
  std::list<Adjacent>::iterator it = std::find_if(m_adjList.begin(),
                                                  m_adjList.end(),
                                                  std::bind(&Adjacent::compareFaceId,_1,faceId));
  if (it != m_adjList.end()) {
    return &(*it);
  }

  return 0;
}

uint64_t
AdjacencyList::getFaceId(const std::string& faceUri)
{
  std::list<Adjacent>::iterator it = std::find_if(m_adjList.begin(),
                                                  m_adjList.end(),
                                                  std::bind(&Adjacent::compareFaceUri,_1,faceUri));
  if (it != m_adjList.end()) {
    return it->getFaceId();
  }

  return 0;
}

void
AdjacencyList::writeLog()
{
  cout << "-------Adjacency List--------"<<endl;
  for (std::list<Adjacent>::iterator it = m_adjList.begin();
       it != m_adjList.end(); it++) {
    (*it).writeLog();
  }
}

} //namespace ndn
} // namespace ns3
