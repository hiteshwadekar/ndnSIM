#include "ndn-controller-router.hpp"
#include "model/ndn-l3-protocol.hpp"
#include "model/ndn-face.hpp"

#include "ns3/channel.h"

namespace ns3 {
namespace ndn {

uint32_t ControllerRouter::m_idCounter = 0;

ControllerRouter::ControllerRouter(std::string &strSourceNode)
{
  m_sourcenode = strSourceNode;
  m_id = m_idCounter;
  m_idCounter++;
}

uint32_t
ControllerRouter::GetId() const
{
  return m_id;
}

Ptr<L3Protocol>
ControllerRouter::GetL3Protocol() const
{
  return m_ndn;
}

void
ControllerRouter::AddLocalPrefix(shared_ptr<Name> prefix)
{
  m_localPrefixes.push_back(prefix);
}

void
ControllerRouter::AddIncidency(shared_ptr<Face> face, Ptr<ControllerRouter> gr)
{
  m_incidencies.push_back(std::make_tuple(this, face, gr));
}

ControllerRouter::IncidencyList&
ControllerRouter::GetIncidencies()
{
  return m_incidencies;
}

const ControllerRouter::LocalPrefixList&
ControllerRouter::GetLocalPrefixes() const
{
  return m_localPrefixes;
}

} // namespace ndn
} // namespace ns3
