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

vector<std::string>
NdnControllerString::GetLinkInfo(){
	std::string strLinkInfo;
	std::vector<std::string> fields;
	if (!m_string.empty())
	{
		strLinkInfo = extractInformation(LINK_INFORMATION,"}");
		fields=extractLinkInformation(strLinkInfo,",");
	}
	return fields;
}

string
NdnControllerString::GetAppPrefixInfo(){
	return m_string;
}

vector<std::string>
NdnControllerString::GetNodePrefixInfo(){
	string strNodePrefixInfo;
	std::vector<std::string> fields;
	if (!m_string.empty())
	{
		strNodePrefixInfo = extractInformation(NODE_PREFIX,"}");
		fields=extractLinkInformation(strNodePrefixInfo,",");
	}
	return fields;
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

std::vector<std::string> NdnControllerString::extractLinkInformation(string key, string strPattern)
{
	std::vector<std::string> fields;
	boost::algorithm::split(fields, key, boost::algorithm::is_any_of(strPattern));
	/*
	for (size_t n = 0; n < fields.size(); n+=1)
	{
		cout << "\n Extracting link/prefix information: "<<endl;
		cout << "\n: " << fields[n] << endl;
	}*/
  return fields;
}


} // namespace ndn
} // namespace ns3
