#include "ndn-controller-string-parser.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/names.h"
#include "apps/ndn-app.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.NdnControllerString");

namespace ns3 {
namespace ndn {

using namespace std;

uint32_t NdnControllerString::m_linkinfoCounter=0;
uint32_t NdnControllerString::m_AppPrefixCounter=0;
uint32_t NdnControllerString::m_NodePrefixCounter=0;

NdnControllerString::NdnControllerString()
{
	m_string="";
}

string
NdnControllerString::GetSubString(int start_pos, int end_pos){
	return m_string.substr (start_pos,end_pos);
}

string
NdnControllerString::stringAppend(string strApString){

	return m_string;
}

string
NdnControllerString::GetSourceNode(){
	return m_string;
}

string
NdnControllerString::GetLinkInfo(){
	return m_string;
}

string
NdnControllerString::GetAppPrefixInfo(){
	return m_string;
}

string
NdnControllerString::GetNodePrefixInfo(){
	return m_string;
}

string
NdnControllerString::SetSourceNode(string strSourceNode){
	if (m_string.empty() and !strSourceNode.empty())
	{
		m_string = "Source_Node_Name:" + strSourceNode + ",";
	}
	return m_string;
}

string
NdnControllerString::SetLinkInfo(string strLinkInfo){
	if(!strLinkInfo.empty())
	{
		m_string = m_string + "Link_Information:{" + strLinkInfo + "}" +"," ;
	}
	return m_string;
}

string
NdnControllerString::SetAppPrefixInfo(string strAppPrefix){
	m_string = m_string + "Node_Prefix:{" + strAppPrefix + "}";
	return m_string;
}

string
NdnControllerString::SetNodePrefixInfo(string strNodePrefix){
	if(!strNodePrefix.empty())
	{
		m_string = m_string + "Node_Prefix:{" + strNodePrefix + "}" +"," ;
	}
	return m_string;
}

string
NdnControllerString::find(std::string strSubString){

	return m_string;
}

} // namespace ndn
} // namespace ns3
