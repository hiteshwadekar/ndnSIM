
#include "ns3/simulator.h"
#include "ns3/object-vector.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "model/ndn-controller-router.hpp"
#include "helper/controller-node-list.hpp"

namespace ns3 {
namespace ndn {

NS_LOG_COMPONENT_DEFINE ("ControllerNodeList");

/**
 * \ingroup network
 * \brief private implementation detail of the ControllerNodeList API.
 */
class ControllerNodeListPriv : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  ControllerNodeListPriv ();
  ~ControllerNodeListPriv ();

  /**
   * \param node node to add
   * \returns index of node in list.
   *
   * This method is called automatically from Node::Node so
   * the user has little reason to call it himself.
   */
  uint32_t Add (Ptr<ControllerRouter> node);

  /**
   * \returns a C++ iterator located at the beginning of this
   *          list.
   */
  ControllerNodeList::Iterator Begin (void) const;

  /**
   * \returns a C++ iterator located at the end of this
   *          list.
   */
  ControllerNodeList::Iterator End (void) const;

  /**
   * \param n index of requested node.
   * \returns the Node associated to index n.
   */
  Ptr<ControllerRouter> GetNode (uint32_t n);

  /**
   * \returns the number of nodes currently in the list.
   */
  uint32_t GetNNodes (void);

  /**
   * \brief Get the node list object
   * \returns the node list
   */
  static Ptr<ControllerNodeListPriv> Get (void);

private:
  /**
   * \brief Get the node list object
   * \returns the node list
   */
  static Ptr<ControllerNodeListPriv> *DoGet (void);

  /**
   * \brief Delete the nodes list object
   */
  static void Delete (void);

  /**
   * \brief Dispose the nodes in the list
   */
  virtual void DoDispose (void);

  std::vector<Ptr<ControllerRouter> > m_nodes; //!< node objects container
};

NS_OBJECT_ENSURE_REGISTERED (ControllerNodeListPriv);

TypeId
ControllerNodeListPriv::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ControllerNodeListPriv")
    .SetParent<Object> ()
    .AddAttribute ("ControllerNodeList", "The list of all nodes created during the simulation.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&ControllerNodeListPriv::m_nodes),
                   MakeObjectVectorChecker<ControllerRouter> ())
  ;
  return tid;
}

Ptr<ControllerNodeListPriv>
ControllerNodeListPriv::Get (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return *DoGet ();
}
Ptr<ControllerNodeListPriv> *
ControllerNodeListPriv::DoGet (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static Ptr<ControllerNodeListPriv> ptr = 0;
  if (ptr == 0)
    {
      ptr = CreateObject<ControllerNodeListPriv> ();
      Config::RegisterRootNamespaceObject (ptr);
    }
  return &ptr;
}
void
ControllerNodeListPriv::Delete (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::UnregisterRootNamespaceObject (Get ());
  (*DoGet ()) = 0;
}


ControllerNodeListPriv::ControllerNodeListPriv ()
{
  NS_LOG_FUNCTION (this);
}
ControllerNodeListPriv::~ControllerNodeListPriv ()
{
  NS_LOG_FUNCTION (this);
}
void
ControllerNodeListPriv::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (std::vector<Ptr<ControllerRouter> >::iterator i = m_nodes.begin ();
       i != m_nodes.end (); i++)
    {
      Ptr<ControllerRouter> node = *i;
      node->Dispose ();
      *i = 0;
    }
  m_nodes.erase (m_nodes.begin (), m_nodes.end ());
  Object::DoDispose ();
}


uint32_t
ControllerNodeListPriv::Add (Ptr<ControllerRouter> node)
{
  NS_LOG_FUNCTION (this << node);
  uint32_t index = m_nodes.size ();
  m_nodes.push_back (node);
  return index;

}
ControllerNodeList::Iterator
ControllerNodeListPriv::Begin (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nodes.begin ();
}
ControllerNodeList::Iterator
ControllerNodeListPriv::End (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nodes.end ();
}
uint32_t
ControllerNodeListPriv::GetNNodes (void)
{
  NS_LOG_FUNCTION (this);
  return m_nodes.size ();
}

Ptr<ControllerRouter>
ControllerNodeListPriv::GetNode (uint32_t n)
{
  NS_LOG_FUNCTION (this << n);
  NS_ASSERT_MSG (n < m_nodes.size (), "Node index " << n <<
                 " is out of range (only have " << m_nodes.size () << " nodes).");
  return m_nodes[n];
}

}
}

/**
 * The implementation of the public static-based API
 * which calls into the private implementation through
 * the simulation singleton.
 */
namespace ns3 {
namespace ndn {

uint32_t
ControllerNodeList::Add (Ptr<ControllerRouter> node)
{
  NS_LOG_FUNCTION (node);
  return ControllerNodeListPriv::Get ()->Add (node);
}
ControllerNodeList::Iterator
ControllerNodeList::Begin (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return ControllerNodeListPriv::Get ()->Begin ();
}
ControllerNodeList::Iterator
ControllerNodeList::End (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return ControllerNodeListPriv::Get ()->End ();
}
Ptr<ControllerRouter>
ControllerNodeList::GetNode (uint32_t n)
{
  NS_LOG_FUNCTION (n);
  return ControllerNodeListPriv::Get ()->GetNode (n);
}
uint32_t
ControllerNodeList::GetNNodes (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return ControllerNodeListPriv::Get ()->GetNNodes ();
}
}
} // namespace ns3
