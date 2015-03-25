#ifndef NDN_CONTROLLER_ROUTER_H
#define NDN_CONTROLLER_ROUTER_H

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/model/ndn-face.hpp"

#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>
#include <tuple>

namespace ns3 {

class Channel;

namespace ndn {

class L3Protocol;
/**
 * @ingroup ndn-helpers
 * @brief Class representing controller router interface for ndnSIM
 */
class ControllerRouter: public Object {
public:
  /**
   * @brief Graph edge
   */
  typedef std::tuple<Ptr<ControllerRouter>, shared_ptr<Face>, Ptr<ControllerRouter>> Incidency;
  /**
   * @brief List of graph edges
   */
  typedef std::list<Incidency> IncidencyList;
  	  /**
   * @brief List of locally exported prefixes
   */
  typedef std::list<shared_ptr<Name>> LocalPrefixList;
  /**
   * @brief Default constructor
   */
  ControllerRouter(std::string &strSourceNode);
  /**
   * @brief Get numeric ID of the node (internally assigned)
   */
  uint32_t
  GetId() const;
  /**
   * @brief Helper function to get smart pointer to ndn::L3Protocol object (basically, self)
   */
  Ptr<L3Protocol>
  GetL3Protocol() const;
  /**
   * @brief Add new locally exported prefix
   * @param prefix Prefix
   */
  void
  AddLocalPrefix(shared_ptr<Name> prefix);
  /**
   * @brief Add edge to the node
   * @param face Face of the edge
   * @param ndn GlobalRouter of another node
   */
  void
  AddIncidency(shared_ptr<Face> face, Ptr<ControllerRouter> ndn);
  /**
   * @brief Get list of edges that are connected to this node
   */
  IncidencyList&
  GetIncidencies();
  /**
   * @brief Get list of locally exported prefixes
   */
  const LocalPrefixList&
  GetLocalPrefixes() const;


private:
  uint32_t m_id;
  std::string m_sourcenode;
  Ptr<L3Protocol> m_ndn;
  LocalPrefixList m_localPrefixes;
  IncidencyList m_incidencies;

  static uint32_t m_idCounter;
};

inline bool
operator==(const ControllerRouter::Incidency& a, const ControllerRouter::Incidency& b)
{
  return std::get<0>(a) == std::get<0>(b) && std::get<1>(a) == std::get<1>(b)
         && std::get<2>(a) == std::get<2>(b);
}

inline bool
operator!=(const ControllerRouter::Incidency& a, const ControllerRouter::Incidency& b)
{
  return !(a == b);
}

} // namespace ndn
} // namespace ns3

#endif // NDN_CONTROLLER_ROUTER_H