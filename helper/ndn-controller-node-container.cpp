#include "ndn-controller-node-container.hpp"
#include "ns3/node-list.h"
#include "ns3/names.h"

namespace ns3 {
namespace ndn{


ControllerNodeContainer::ControllerNodeContainer ()
{
}

ControllerNodeContainer::ControllerNodeContainer (Ptr<ControllerRouter> node)
{
  m_nodes.push_back (node);
}
ControllerNodeContainer::ControllerNodeContainer (std::string nodeName)
{
  Ptr<ControllerRouter> node = Names::Find<ControllerRouter> (nodeName);
  m_nodes.push_back (node);
}
ControllerNodeContainer::ControllerNodeContainer (const ControllerNodeContainer &a, const ControllerNodeContainer &b)
{
  Add (a);
  Add (b);
}
ControllerNodeContainer::ControllerNodeContainer (const ControllerNodeContainer &a, const ControllerNodeContainer &b,
                              const ControllerNodeContainer &c)
{
  Add (a);
  Add (b);
  Add (c);
}
ControllerNodeContainer::ControllerNodeContainer (const ControllerNodeContainer &a, const ControllerNodeContainer &b,
                              const ControllerNodeContainer &c, const ControllerNodeContainer &d)
{
  Add (a);
  Add (b);
  Add (c);
  Add (d);
}

ControllerNodeContainer::ControllerNodeContainer (const ControllerNodeContainer &a, const ControllerNodeContainer &b,
                              const ControllerNodeContainer &c, const ControllerNodeContainer &d,
                              const ControllerNodeContainer &e)
{
  Add (a);
  Add (b);
  Add (c);
  Add (d);
  Add (e);
}

ControllerNodeContainer::Iterator
ControllerNodeContainer::Begin (void) const
{
  return m_nodes.begin ();
}

ControllerNodeContainer::const_iterator
ControllerNodeContainer::begin () const
{
  return m_nodes.begin ();
}

ControllerNodeContainer::iterator
ControllerNodeContainer::begin ()
{
  return m_nodes.begin ();
}


ControllerNodeContainer::Iterator
ControllerNodeContainer::End (void) const
{
  return m_nodes.end ();
}

ControllerNodeContainer::const_iterator
ControllerNodeContainer::end () const
{
  return m_nodes.end ();
}

ControllerNodeContainer::iterator
ControllerNodeContainer::end ()
{
  return m_nodes.end ();
}

uint32_t
ControllerNodeContainer::GetN (void) const
{
  return m_nodes.size ();
}

uint32_t
ControllerNodeContainer::size () const
{
  return m_nodes.size ();
}

Ptr<ControllerRouter>
ControllerNodeContainer::Get (uint32_t i) const
{
  return m_nodes[i];
}

Ptr<ControllerRouter> &
ControllerNodeContainer::operator [] (uint32_t i)
{
  return m_nodes [i];
}

void
ControllerNodeContainer::Add (ControllerNodeContainer other)
{
  for (Iterator i = other.Begin (); i != other.End (); i++)
    {
      m_nodes.push_back (*i);
    }
}
void
ControllerNodeContainer::Add (Ptr<ControllerRouter> node)
{
  m_nodes.push_back (node);
}
void
ControllerNodeContainer::Add (std::string nodeName)
{
  Ptr<ControllerRouter> node = Names::Find<ControllerRouter> (nodeName);
  m_nodes.push_back (node);
}

/*
ControllerNodeContainer
ControllerNodeContainer::GetGlobal (void)
{
  ControllerNodeContainer c;
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
    {
      c.Add (*i);
    }
  return c;
}*/
}  // namespace ndn
} // namespace ns3
