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

NdnControllerString::NdnControllerString(string strInitial)
{
	m_string=strInitial;
}

string
NdnControllerString::GetSubString(int start_pos, int end_pos){
	return m_string.substr (start_pos,end_pos);
}

string
NdnControllerString::GetString(){
	return m_string;
}


string
NdnControllerString::stringAppend(string strApString){

	return m_string;
}

string
NdnControllerString::GetSourceNode(){
	string strSourceNode;
	if (!m_string.empty())
	{
		strSourceNode = extractInformation(SOURCE_NODE_NAME,",");
	}
	return strSourceNode;
}

string
NdnControllerString::GetLinkInfo(){
	string strLinkInfo;
	if (!m_string.empty())
	{
		strLinkInfo = extractInformation(LINK_INFORMATION,"}");
	}
	return strLinkInfo;
}

string
NdnControllerString::GetAppPrefixInfo(){
	return m_string;
}

string
NdnControllerString::GetNodePrefixInfo(){
	string strNodePrefixInfo;
	if (!m_string.empty())
	{
		strNodePrefixInfo = extractInformation(NODE_PREFIX,"}");
	}
	return strNodePrefixInfo;
}

string
NdnControllerString::SetSourceNode(string strSourceNode){
	if (m_string.empty() and !strSourceNode.empty())
	{
		m_string = SOURCE_NODE_NAME + strSourceNode + ",";
	}
	return m_string;
}

string
NdnControllerString::SetLinkInfo(string strLinkInfo){
	if(!strLinkInfo.empty())
	{
		m_string = m_string + LINK_INFORMATION + strLinkInfo + "}" +"," ;
	}
	return m_string;
}

string
NdnControllerString::SetAppPrefixInfo(string strAppPrefix){
	m_string = m_string + APP_PREFIX + strAppPrefix + "}";
	return m_string;
}

string
NdnControllerString::SetNodePrefixInfo(string strNodePrefix){
	if(!strNodePrefix.empty())
	{
		m_string = m_string + NODE_PREFIX + strNodePrefix + "}" +"," ;
	}
	return m_string;
}

string NdnControllerString::extractInformation(string key, string strPattern)
{
  string s = key;
  size_t p1 = m_string.find(s);
  if (string::npos != p1)
    p1 += s.size();
  size_t p2 = m_string.find_first_of(strPattern, p1);
  if (string::npos != p2)
    return m_string.substr(p1,p2-p1);
  return "";
}

} // namespace ndn
} // namespace ns3
