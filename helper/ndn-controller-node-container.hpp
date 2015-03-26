#ifndef NDN_CONROLLER_NODE_CONTAINER_H
#define NDN_CONROLLER_NODE_CONTAINER_H

#include <stdint.h>
#include <vector>
#include "model/ndn-controller-router.hpp"

namespace ns3 {
namespace ndn {
/**
 * \brief keep track of a set of node pointers.
 *
 * Typically ns-3 helpers operate on more than one node at a time.  For example
 * a device helper may want to install devices on a large number of similar
 * nodes.  The helper Install methods usually take a ControllerNodeContainer as a
 * parameter.  ControllerNodeContainers hold the multiple Ptr<ControllerRouter> which are used
 * to refer to the nodes.
 */
class ControllerNodeContainer
{
public:
  /// ControllerRouter container iterator
  typedef std::vector<Ptr<ControllerRouter> >::const_iterator Iterator;
  typedef std::vector<Ptr<ControllerRouter> >::iterator iterator;
  typedef std::vector<Ptr<ControllerRouter> >::const_iterator const_iterator;

  /**
   * Create an empty ControllerControllerNodeContainer.
   */
  ControllerNodeContainer ();

  /**
   * Create a ControllerControllerNodeContainer with exactly one node which has been previously
   * instantiated.  The single ControllerRouter is specified by a smart pointer.
   *
   * \param node The Ptr<ControllerRouter> to add to the container.
   */
  ControllerNodeContainer (Ptr<ControllerRouter> node);

  /**
   * Create a ControllerControllerNodeContainer with exactly one node which has been previously
   * instantiated and assigned a name using the Object Name Service.  This
   * ControllerRouter is then specified by its assigned name.
   *
   * \param nodeName The name of the ControllerRouter Object to add to the container.
   */
  ControllerNodeContainer (std::string nodeName);

  /**
   * Create a node container which is a concatenation of two input
   * ControllerNodeContainers.
   *
   * \param a The first ControllerNodeContainer
   * \param b The second ControllerNodeContainer
   *
   * \note A frequently seen idiom that uses these constructors involves the
   * implicit conversion by constructor of Ptr<ControllerRouter>.  When used, two
   * Ptr<ControllerRouter> will be passed to this constructor instead of ControllerNodeContainer&.
   * C++ will notice the implicit conversion path that goes through the
   * ControllerNodeContainer (Ptr<ControllerRouter> node) constructor above.  Using this conversion
   * one may provide optionally provide arguments of Ptr<ControllerRouter> to these
   * constructors.
   */
  ControllerNodeContainer(const ControllerNodeContainer &a, const ControllerNodeContainer &b);

  /**
   * Create a node container which is a concatenation of three input
   * ControllerNodeContainers.
   *
   * \param a The first ControllerNodeContainer
   * \param b The second ControllerNodeContainer
   * \param c The third ControllerNodeContainer
   *
   * \note A frequently seen idiom that uses these constructors involves the
   * implicit conversion by constructor of Ptr<ControllerRouter>.  When used, two
   * Ptr<ControllerRouter> will be passed to this constructor instead of ControllerNodeContainer&.
   * C++ will notice the implicit conversion path that goes through the
   * ControllerNodeContainer (Ptr<ControllerRouter> node) constructor above.  Using this conversion
   * one may provide optionally provide arguments of Ptr<ControllerRouter> to these
   * constructors.
   */
  ControllerNodeContainer(const ControllerNodeContainer &a, const ControllerNodeContainer &b, const ControllerNodeContainer &c);

  /**
   * Create a node container which is a concatenation of four input
   * ControllerNodeContainers.
   *
   * \param a The first ControllerNodeContainer
   * \param b The second ControllerNodeContainer
   * \param c The third ControllerNodeContainer
   * \param d The fourth ControllerNodeContainer
   *
   * \note A frequently seen idiom that uses these constructors involves the
   * implicit conversion by constructor of Ptr<ControllerRouter>.  When used, two
   * Ptr<ControllerRouter> will be passed to this constructor instead of ControllerNodeContainer&.
   * C++ will notice the implicit conversion path that goes through the
   * ControllerNodeContainer (Ptr<ControllerRouter> node) constructor above.  Using this conversion
   * one may provide optionally provide arguments of Ptr<ControllerRouter> to these
   * constructors.
   */
  ControllerNodeContainer (const ControllerNodeContainer &a, const ControllerNodeContainer &b, const ControllerNodeContainer &c, const ControllerNodeContainer &d);

  /**
   * Create a node container which is a concatenation of five input
   * ControllerNodeContainers.
   *
   * \param a The first ControllerNodeContainer
   * \param b The second ControllerNodeContainer
   * \param c The third ControllerNodeContainer
   * \param d The fourth ControllerNodeContainer
   * \param e The fifth ControllerNodeContainer
   *
   * \note A frequently seen idiom that uses these constructors involves the
   * implicit conversion by constructor of Ptr<ControllerRouter>.  When used, two
   * Ptr<ControllerRouter> will be passed to this constructor instead of ControllerNodeContainer&.
   * C++ will notice the implicit conversion path that goes through the
   * ControllerNodeContainer (Ptr<ControllerRouter> node) constructor above.  Using this conversion
   * one may provide optionally provide arguments of Ptr<ControllerRouter> to these
   * constructors.
   */
  ControllerNodeContainer (const ControllerNodeContainer &a, const ControllerNodeContainer &b, const ControllerNodeContainer &c, const ControllerNodeContainer &d,
                 const ControllerNodeContainer &e);

  /**
   * \brief Get an iterator which refers to the first ControllerRouter in the
   * container.
   *
   * Nodes can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the iterator method and is typically used in a
   * for-loop to run through the Nodes
   *
   * \code
   *   ControllerNodeContainer::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       (*i)->method ();  // some ControllerRouter method
   *     }
   * \endcode
   *
   * \returns an iterator which refers to the first ControllerRouter in the container.
   */
  Iterator Begin (void) const;

  const_iterator begin () const; /// @brief STL-like method, @see Begin
  iterator begin (); /// @brief STL-like method, @see Begin

  /**
   * \brief Get an iterator which indicates past-the-last ControllerRouter in the
   * container.
   *
   * Nodes can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the iterator method and is typically used in a
   * for-loop to run through the Nodes
   *
   * \code
   *   ControllerNodeContainer::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       (*i)->method ();  // some ControllerRouter method
   *     }
   * \endcode
   *
   * \returns an iterator which indicates an ending condition for a loop.
   */
  Iterator End (void) const;

  const_iterator end () const; /// @brief STL-like method, @see End
  iterator end (); /// @brief STL-like method, @see End

  /**
   * \brief Get the number of Ptr<ControllerRouter> stored in this container.
   *
   * Nodes can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the direct method and is typically used to
   * define an ending condition in a for-loop that runs through the stored
   * Nodes
   *
   * \code
   *   uint32_t nNodes = container.GetN ();
   *   for (uint32_t i = 0 i < nNodes; ++i)
   *     {
   *       Ptr<ControllerRouter> p = container.Get (i)
   *       i->method ();  // some ControllerRouter method
   *     }
   * \endcode
   *
   * \returns the number of Ptr<ControllerRouter> stored in this container.
   */
  uint32_t GetN (void) const;

  uint32_t size () const; /// @brief STL-like method, @see GetN

  /**
   * \brief Get the Ptr<ControllerRouter> stored in this container at a given
   * index.
   *
   * Nodes can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the direct method and is used to retrieve the
   * indexed Ptr<Appliation>.
   *
   * \code
   *   uint32_t nNodes = container.GetN ();
   *   for (uint32_t i = 0 i < nNodes; ++i)
   *     {
   *       Ptr<ControllerRouter> p = container.Get (i)
   *       i->method ();  // some ControllerRouter method
   *     }
   * \endcode
   *
   * \param i the index of the requested node pointer.
   * \returns the requested node pointer.
   */
  Ptr<ControllerRouter>
  Get (uint32_t i) const;

  Ptr<ControllerRouter> &
  operator [] (uint32_t i); /// @brief STL-like method, @see Get

  /**
   * \brief Create n nodes and append pointers to them to the end of this
   * ControllerNodeContainer.
   *
   * Nodes are at the heart of any ns-3 simulation.  One of the first tasks that
   * any simulation needs to do is to create a number of nodes.  This method
   * automates that task.
   *
   * \param n The number of Nodes to create
   */
  void Create (uint32_t n);

  /**
   * \brief Create n nodes with specified systemId for distributed simulations
   * and append pointers to them to the end of this ControllerNodeContainer.
   *
   * Nodes are at the heart of any ns-3 simulation.  One of the first tasks that
   * any simulation needs to do is to create a number of nodes.  This method
   * automates that task, and adds the ability to specify systemId for
   * distributed simulations.
   *
   * \param n The number of Nodes to create
   * \param systemId The system id or rank associated with this node
   */
  void Create (uint32_t n, uint32_t systemId);

  /**
   * \brief Append the contents of another ControllerNodeContainer to the end of
   * this container.
   *
   * \param other The ControllerNodeContainer to append.
   */
  void Add (ControllerNodeContainer other);

  /**
   * \brief Append a single Ptr<ControllerRouter> to this container.
   *
   * \param node The Ptr<ControllerRouter> to append.
   */
  void Add (Ptr<ControllerRouter> node);

  /**
   * \brief Append to this container the single Ptr<ControllerRouter> referred to
   * via its object name service registered name.
   *
   * \param nodeName The name of the ControllerRouter Object to add to the container.
   */
  void Add (std::string nodeName);

  /**
   * \brief Create a ControllerNodeContainer that contains a list of _all_ nodes
   * created through ControllerNodeContainer::Create() and stored in the
   * ns3::NodeList.
   *
   * Whenever a ControllerRouter is created, a Ptr<ControllerRouter> is added to a global list of all
   * nodes in the system.  It is sometimes useful to be able to get to all
   * nodes in one place.  This method creates a ControllerNodeContainer that is
   * initialized to contain all of the simulation nodes,
   *
   * \returns a NoceContainer which contains a list of all Nodes.
   */
  static ControllerNodeContainer GetGlobal (void);

private:
  std::vector<Ptr<ControllerRouter> > m_nodes; //!< Nodes smart pointers
};

}
} // namespace ns3

#endif /* NDN_CONROLLER_NODE_CONTAINER_H */
