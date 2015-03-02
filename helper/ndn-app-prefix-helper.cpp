#include "ndn-app-prefix-helper.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/names.h"
#include "model/ndn-l3-protocol.hpp"

#include "apps/ndn-app.hpp"


NS_LOG_COMPONENT_DEFINE("ndn.AppPrefixHelper");

namespace ns3 {
namespace ndn {

uint32_t AppPrefixHelper::m_idCounter = 0;

NS_OBJECT_ENSURE_REGISTERED(AppPrefixHelper);

TypeId
AppPrefixHelper::GetTypeId()
{
  static TypeId tid = TypeId("ns3::ndn::AppPrefixHelper").SetGroupName("Ndn").SetParent<Object>();
  return tid;
}

AppPrefixHelper::AppPrefixHelper()
{
  m_id = m_idCounter;
  m_idCounter++;
}

void
AppPrefixHelper::NotifyNewAggregate()
{
  if (m_ndn == 0) {
    m_ndn = GetObject<L3Protocol>();
  }
  Object::NotifyNewAggregate();
}

uint32_t
AppPrefixHelper::GetId() const
{
  return m_id;
}

Ptr<L3Protocol>
AppPrefixHelper::GetL3Protocol() const
{
  return m_ndn;
}

void
AppPrefixHelper::SetMap(TypeId m_tid, std::list<std::string> m_prxlist)
{
	m_prefixmap[m_tid]=m_prxlist;
}

std::map<TypeId, std::list<std::string>>
AppPrefixHelper::GetMap()
{
  return m_prefixmap;
}

} // namespace ndn
} // namespace ns3
