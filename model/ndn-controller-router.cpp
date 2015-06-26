#include "ndn-controller-router.hpp"
#include "model/ndn-face.hpp"

#include <exception>

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
	m_nID = m_idCounter;
	m_idCounter++;
}


ControllerRouter::ControllerRouter(std::string& strSourceNode) {
	m_sourcenode = strSourceNode;
	m_status = false;
	m_id = m_idCounter;
	m_nID = m_idCounter;
	m_idCounter++;
}

TypeId ControllerRouter::GetTypeId() {
	static TypeId tid =
			TypeId("ns3::ndn::ControllerRouter").SetGroupName("Ndn").SetParent<
					Object>();
	return tid;
}

uint32_t ControllerRouter::GetId() const {
	return m_id;
}


void ControllerRouter::NotifyNewAggregate() {
	Object::NotifyNewAggregate();
}

void ControllerRouter::AddLocalPrefix(shared_ptr<Name> prefix) {
	m_localPrefixes.push_back(prefix);
}

void ControllerRouter::AddIncidency(shared_ptr<Face> faceId,
		Ptr<ControllerRouter> gr, size_t faceMetrics) {
	m_incidencies.push_back(std::make_tuple(this, faceId, gr, faceMetrics));
	m_status = true;
}

bool ControllerRouter::UpdateIncidency(shared_ptr<Face> faceId,
	Ptr<ControllerRouter> gr, size_t faceMetrics){
	bool isUpdate=false;
	std::list<std::tuple<Ptr<ControllerRouter>, shared_ptr<Face>, Ptr<ControllerRouter>, size_t>>::iterator iter;
	for (iter = m_incidencies.begin();iter!=m_incidencies.end();iter++)
	{
		if(gr == std::get<2>(*iter) && faceId == std::get<1>(*iter))
		{
			std::get<3>(*iter) == faceMetrics;
			isUpdate=true;
			break;
		}
	}
	return isUpdate;
}

bool ControllerRouter::RemoveIncidency(shared_ptr<Face> faceId,
		Ptr<ControllerRouter> gr, size_t faceMetrics){
	bool isUpdate=false;
	std::list<std::tuple<Ptr<ControllerRouter>, shared_ptr<Face>, Ptr<ControllerRouter>, size_t>>::iterator iter;
	for (iter = m_incidencies.begin();iter!=m_incidencies.end();iter++)
	{
		if(gr == std::get<2>(*iter) && faceId == std::get<1>(*iter))
		{
			//m_incidencies.erase(iter);
			m_incidencies.remove(std::make_tuple(this, faceId, gr, faceMetrics));
			isUpdate=true;
			break;
		}
	}
	return isUpdate;
}

void ControllerRouter::AddMultiPathIncidency(shared_ptr<Face> faceId,
		Ptr<ControllerRouter> gr, size_t faceMetrics) {
	m_multiPath_incidencies.push_back(std::make_tuple(this, faceId, gr, faceMetrics));
}

void ControllerRouter::AddMultiPathIncidencies(std::list<Incidency>& lstIncidencies) {
	m_multiPath_incidencies=lstIncidencies;
}

void ControllerRouter::
 ResetMultiPathIncidencies(){
	m_multiPath_incidencies.clear();
}

void ControllerRouter::AddPaths(Ptr<ControllerRouter> ndn, std::list<std::tuple<std::shared_ptr<Name>,std::shared_ptr<Face>,size_t>> lstPath) {

	if ( m_pathInfoList.find(ndn) == m_pathInfoList.end() )
	{
		m_pathInfoList[ndn]=lstPath;
	}
	else
	{
		m_pathInfoList[ndn].splice(m_pathInfoList[ndn].end(),lstPath);
	}
}

const ControllerRouter::PathInfo&
ControllerRouter::GetPathInfo() const {
	return m_pathInfoList;
}

ControllerRouter::IncidencyList&
ControllerRouter::GetIncidencies() {
	return m_incidencies;
}

ControllerRouter::MultiPathIncidencyList&
ControllerRouter::GetMultiPathIncidencies() {
	return m_multiPath_incidencies;
}

const ControllerRouter::LocalPrefixList&
ControllerRouter::GetLocalPrefixes() const {
	return m_localPrefixes;
}

std::string&
ControllerRouter::GetSourceNode() {
	return m_sourcenode;
}

bool ControllerRouter::GetStatus() {
	return m_status;
}

void ControllerRouter::PrintInfo() {
	std::cout << "SourceNode name -> " << m_sourcenode << std::endl;
}

} // namespace ndn
} // namespace ns3
