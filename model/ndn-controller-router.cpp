#include "ndn-controller-router.hpp"
#include "model/ndn-face.hpp"

#include <list>
#include <tuple>

namespace ns3 {
namespace ndn {

uint32_t ControllerRouter::m_idCounter = 1;
bool ControllerRouter::m_status = false;
NS_OBJECT_ENSURE_REGISTERED(ControllerRouter);

ControllerRouter::ControllerRouter()
{
	m_id = m_idCounter;
	m_idCounter++;
}

ControllerRouter::ControllerRouter(std::string& strSourceNode)
{
  m_sourcenode = strSourceNode;
  m_status = false;
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
ControllerRouter::AddIncidency(shared_ptr<Face> faceId, Ptr<ControllerRouter> gr, size_t faceMetrics)
{
  m_incidencies.push_back(std::make_tuple(this, faceId, gr, faceMetrics));
  m_status=true;
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

std::string&
ControllerRouter::GetSourceNode()
{
  return m_sourcenode;
}

bool
ControllerRouter::GetStatus()
{
  return m_status;
}

void
ControllerRouter::PrintInfo()
{
	std::cout << "SourceNode name -> " << m_sourcenode << std::endl;
}

} // namespace ndn
} // namespace ns3
