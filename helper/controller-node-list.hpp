#ifndef CONTROLLER_NODE_LIST_H
#define CONTROLLER_NODE_LIST_H

#include <vector>
#include "ns3/ptr.h"
#include "model/ndn-controller-router.hpp"

namespace ns3 {
namespace ndn {

class ControllerRouter;
class CallbackBase;

/**
 * \ingroup network
 *
 * \brief the list of simulation nodes.
 *
 * Every Node created is automatically added to this list.
 */
class ControllerNodeList
{
public:
  /// Node container iterator
  typedef std::vector< Ptr<ControllerRouter> >::const_iterator Iterator;

  /**
   * \param node node to add
   * \returns index of node in list.
   *
   * This method is called automatically from Node::Node so
   * the user has little reason to call it himself.
   */
  static uint32_t Add (Ptr<ControllerRouter> node);
  /**
   * \returns a C++ iterator located at the beginning of this
   *          list.
   */
  static Iterator Begin (void);
  /**
   * \returns a C++ iterator located at the end of this
   *          list.
   */
  static Iterator End (void);
  /**
   * \param n index of requested node.
   * \returns the Node associated to index n.
   */
  static Ptr<ControllerRouter> GetNode (uint32_t n);
  /**
   * \returns the number of nodes currently in the list.
   */
  static uint32_t GetNNodes (void);
};
} // namespace ndn
} // namespace ns3

#endif /* CONTROLLER_NODE_LIST_H */
