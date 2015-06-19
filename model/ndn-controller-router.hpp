#ifndef NDN_CONTROLLER_ROUTER_H
#define NDN_CONTROLLER_ROUTER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/model/ndn-face.hpp"

#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>
#include <tuple>

namespace ns3 {
namespace ndn {
/**
 * @ingroup ndn-helpers
 * @brief Class representing controller router interface for ndnSIM
 */
class ControllerRouter: public Object {
public:
  /**
   * @brief Graph edge
   */
  typedef std::tuple<Ptr<ControllerRouter>, shared_ptr<Face>, Ptr<ControllerRouter>, size_t> Incidency;
  /**
   * @brief List of total graph edges
   */
  typedef std::list<Incidency> IncidencyList;
  /**
  * @brief List of graph multipath active edges
    */
   typedef std::list<Incidency> MultiPathIncidencyList;
  /**
   * @brief List of locally exported prefixes
   */
  typedef std::list<shared_ptr<Name>> LocalPrefixList;
  /**
   * @brief Graph edge
   */
  typedef std::map<Ptr<ControllerRouter>,std::list<std::tuple<shared_ptr<Name>,shared_ptr<Face>,size_t>>> PathInfo;
  /**
       * @brief List of graph edges
       */
  typedef std::list<PathInfo> PathInfoList;
  /**
   * \brief Interface ID
   *
   * \return interface ID
   */
  static TypeId
  GetTypeId();
  /**
    * @brief Get numeric ID of the node (internally assigned)
    */
  uint32_t
  GetId() const;
  /**
   * @brief Default constructor
   */
  ControllerRouter();

  ControllerRouter(std::string& strSourceNode);
  /**
   * @brief Add new locally exported prefix
   * @param prefix Prefix
   */
  void
  AddLocalPrefix(shared_ptr<Name> prefix);
  /**
   * @brief Add edge to the node
   * @param face Face of the edge
   * @param ndn ControllerRouter of another node
   */
  void
  AddIncidency(shared_ptr<Face> faceId, Ptr<ControllerRouter> ndn, size_t faceMetric);
  /**
   * @brief update edge to the node
   * @param face Face of the edge
   * @param ndn ControllerRouter of another node
   */
  bool
  UpdateIncidency(shared_ptr<Face> faceId, Ptr<ControllerRouter> ndn, size_t faceMetric);
  /**
   * @brief Remove edge to the node
   * @param face Face of the edge
   * @param ndn ControllerRouter of another node
   */
  bool
  RemoveIncidency(shared_ptr<Face> faceId, Ptr<ControllerRouter> gr, size_t faceMetrics);
  /**
   * @brief Add edge to the node
   * @param face Face of the edge
   * @param ndn ControllerRouter of another node
   */
  void
  AddMultiPathIncidency(shared_ptr<Face>,Ptr<ControllerRouter>,size_t);
  /**
   * @brief Add edge to the node
   * @param face Face of the edge
   * @param ndn ControllerRouter of another node
   */
  void
  AddMultiPathIncidencies(std::list<Incidency>&);
  /**
   * @brief Add calculated path to the node
   * @param prefix name
   * @param face Face of the edge
   * @param calculated metrics of the node.
   */
  void
  AddPaths(Ptr<ControllerRouter> ndn,std::list<std::tuple<shared_ptr<Name>,shared_ptr<Face>,size_t>> lstPath);
  /**
   * @brief Get list of edges that are connected to this node
   */
  IncidencyList&
  GetIncidencies();
  /**
   * @brief Get list of edges that are connected to this node
  */
  MultiPathIncidencyList&
  GetMultiPathIncidencies();
  /**
   *
   */
  void
  ResetMultiPathIncidencies();
  /**
   * @brief Get list of locally exported prefixes
   */
  const LocalPrefixList&
  GetLocalPrefixes() const;

  std::string&
  GetSourceNode();

  bool
  GetStatus();

  void
  PrintInfo();

  const PathInfo&
  GetPathInfo() const;

  void LinkInitalization(Ptr<ControllerRouter>,shared_ptr<Face>,Ptr<ControllerRouter>);

  IncidencyList&
  FindIncidency();


  // Information from BaseVertex
  int getID() const { return m_nID; }
  void setID(int ID_) { m_nID = ID_; }

  double Weight() const { return m_dWeight; }
  void Weight(double val) { m_dWeight = val; }

  void PrintOut(std::ostream& out_stream)
  {
  	//out_stream << m_nID;
  	std::cout << m_sourcenode;
  }

  // ??
protected:
  virtual void
  NotifyNewAggregate(); ///< @brief Notify when the object is aggregated to another object (e.g.,
                       /// Node)
private:
  uint32_t m_id;
  static bool m_status;
  std::string m_sourcenode;
  LocalPrefixList m_localPrefixes;
  IncidencyList m_incidencies;
  MultiPathIncidencyList m_multiPath_incidencies;
  PathInfo m_pathInfoList;
  static uint32_t m_idCounter;

  //For Yan's K path alorithm
  int m_nID;
  double m_dWeight;
};

inline bool
operator==(const ControllerRouter::Incidency& a, const ControllerRouter::Incidency& b)
{
  return std::get<0>(a) == std::get<0>(b) && std::get<1>(a) == std::get<1>(b)
         && std::get<2>(a) == std::get<2>(b) && std::get<3>(a) == std::get<3>(b);
}

inline bool
operator!=(const ControllerRouter::Incidency& a, const ControllerRouter::Incidency& b)
{
  return !(a == b);
}

} // namespace ndn
} // namespace ns3

#endif // NDN_CONTROLLER_ROUTER_H
