#ifndef NDN_ADJACENCY_LIST_HPP
#define NDN_ADJACENCY_LIST_HPP

#include <list>
#include <map>
#include <boost/cstdint.hpp>
#include "ndn-adjacency.hpp"
#include "ndn-conf-parameter.hpp"

namespace ns3{
namespace ndn{

class AdjacencyList
{

public:
  AdjacencyList();
  ~AdjacencyList();

  int32_t
  getRetryPacketCount(const Name& adjName);

  bool
  insertRetryPacketCount(const Name& adjName, int32_t count);

  void
  resetRetryPacketZero();

  bool
  incrementRetryPacketCount(const Name& adjName);

  int32_t
  insert(Adjacent& adjacent);

  bool
  updateAdjacentStatus(const Name& adjName, Adjacent::Status s);

  int32_t
  updateAdjacentLinkCost(const Name& adjName, double lc);

  std::list<Adjacent>&
  getAdjList();

  void
  setAdjList(std::list<Adjacent>&);

  bool
  isNeighbor(const Name& adjName);

  void
  incrementTimedOutInterestCount(const Name& neighbor);

  int32_t
  getTimedOutInterestCount(const Name& neighbor);

  Adjacent::Status
  getStatusOfNeighbor(const Name& neighbor);

  void
  setStatusOfNeighbor(const Name& neighbor, Adjacent::Status status);

  void
  setTimedOutInterestCount(const Name& neighbor, uint32_t count);

  void
  incrementInterestSendCount(const Name& neighbor);

  void
  setInterestSendCount(const Name& neighbor,
                                          uint32_t count);
  int32_t
  getInterestSendCount(const Name& neighbor);

  void
  setDataRcvCount(const Name& neighbor, uint32_t count);

  void
  incrementDataRcvCount(const Name& neighbor);

  int32_t
  getDataRcvCount(const Name& neighbor);

  void
  addAdjacents(AdjacencyList& adl);

  bool
  isAdjBuildable();

  int32_t
  getNumOfActiveNeighbor();

  Adjacent
  getAdjacent(const Name& adjName);

  bool
  operator==(AdjacencyList& adl);

  size_t
  getSize()
  {
    return m_adjList.size();
  }

  void
  reset()
  {
    if (m_adjList.size() > 0) {
      m_adjList.clear();
    }
  }

  void
  resetRetryPacketCount()
  {
	  if(m_retryPacketInfo.size()> 0){
		  m_retryPacketInfo.clear();
	  }
  }

  Adjacent*
  findAdjacent(const Name& adjName);

  Adjacent*
  findAdjacent(uint64_t faceId);

  uint64_t
  getFaceId(const std::string& faceUri);

  void
  writeLog();

private:
  std::list<Adjacent>::iterator
  find(const Name& adjName);

private:
  std::list<Adjacent> m_adjList;
  std::map<Name,int32_t> m_retryPacketInfo;
  ConfParameter m_conf;
};

} //namespace ndn
} //namespace ns3
#endif //NDN_ADJACENCY_LIST_HPP
