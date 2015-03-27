#include "ndn-controller-router.hpp"
#include "model/ndn-face.hpp"

#include <list>
#include <tuple>

namespace ns3 {
namespace ndn {

uint32_t ControllerRouter::m_idCounter = 0;
NS_OBJECT_ENSURE_REGISTERED(ControllerRouter);

ControllerRouter::ControllerRouter()
{
	m_id = m_idCounter;
	m_idCounter++;
}

ControllerRouter::ControllerRouter(std::string& strSourceNode)
{
  m_sourcenode = strSourceNode;
  m_id = m_idCounter;
  m_idCounter++;
}

TypeId
ControllerRouter::GetTypeId()
{
  static TypeId tid = TypeId("ns3::ndn::ControllerRouter").SetGroupName("Ndn").SetParent<Object>();
  return tid;
}

uint32_t
ControllerRouter::GetId() const
{
  return m_id;
}

void
ControllerRouter::NotifyNewAggregate()
{
  Object::NotifyNewAggregate();
}

void
ControllerRouter::AddLocalPrefix(shared_ptr<Name> prefix)
{
  m_localPrefixes.push_back(prefix);
}

void
ControllerRouter::AddIncidency(shared_ptr<size_t> faceId, Ptr<ControllerRouter> gr)
{
  m_incidencies.push_back(std::make_tuple(this, faceId, gr));
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
