#include "ndn-controllerapp.hpp"
#include "ns3/log.h"

//#include "ns3/ndn-interest.hpp"
//#include "ns3/ndn-data.hpp"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/double.h"

//#include "ns3/ndn-app-face.h"
//#include "ns3/ndn-fib.hpp"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"
#include "model/ndn-net-device-face.hpp"

#include <memory>

#include "daemon/table/fib.hpp"
#include "daemon/fw/forwarder.hpp"
#include "daemon/table/fib-entry.hpp"
#include "daemon/table/fib-nexthop.hpp"

#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/names.h"
#include "ns3/node-list.h"
#include "ns3/channel-list.h"
#include "ns3/object-factory.h"


#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/concept/assert.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include "helper/boost-graph-ndn-controller-routing-helper.hpp"
#include "helper/controller-node-list.hpp"

#include "helper/ndn-app-prefix-helper.hpp"

#include <boost/ref.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <typeinfo>

namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE("ndn.ControllerApp");

using namespace std;


namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ControllerApp);

TypeId ControllerApp::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::ndn::ControllerApp").SetGroupName("Ndn").SetParent<App>().AddConstructor<
					ControllerApp>().AddAttribute("Prefix",
					"Prefix, Controller prefix information", StringValue("/"),
					MakeNameAccessor(&ControllerApp::m_prefix),
					MakeNameChecker())
					.AddAttribute("Frequency",
					"Frequency of interest packets", StringValue("1.0"),
					MakeDoubleAccessor(&ControllerApp::m_frequency),
					MakeDoubleChecker<double>()).AddAttribute("PayloadSize",
					"Virtual payload size for Content packets",
					UintegerValue(1024),
					MakeUintegerAccessor(&ControllerApp::m_virtualPayloadSize),
					MakeUintegerChecker<uint32_t>()).AddAttribute("Freshness",
					"Freshness of data packets, if 0, then unlimited freshness",
					TimeValue(Seconds(0)),
					MakeTimeAccessor(&ControllerApp::m_freshness),
					MakeTimeChecker()).AddAttribute("Signature",
					"Fake signature, 0 valid signature (default), other values application-specific",
					UintegerValue(0),
					MakeUintegerAccessor(&ControllerApp::m_signature),
					MakeUintegerChecker<uint32_t>()).AddAttribute("KeyLocator",
					"Name to be used for key locator.  If root, then key locator is not used",
					NameValue(), MakeNameAccessor(&ControllerApp::m_keyLocator),
					MakeNameChecker());
	return tid;
}

ControllerApp::ControllerApp() {
	// NS_LOG_FUNCTION_NOARGS ();
}

// inherited from Application base class.
void ControllerApp::StartApplication() {
	NS_LOG_FUNCTION_NOARGS ();
	App::StartApplication();
	NS_LOG_DEBUG("NodeID: " << GetNode ()->GetId ());
	FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
}

void ControllerApp::StopApplication() {
	NS_LOG_FUNCTION_NOARGS ();
	App::StopApplication();
}

std::string ControllerApp::extractNodeName(std::string strPacketName, int n) {

	std::cout << strPacketName;
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strPacketName,
			boost::algorithm::is_any_of("/"));
	return fields[n];
}

std::string ControllerApp::extractNodeRequestType(std::string strPrefixName) {
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strPrefixName,
			boost::algorithm::is_any_of("/"));
	return fields[3];
}


void ControllerApp::extractNodeLinkInfo(std::string strNodeLinkInfo) {
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strNodeLinkInfo,
				boost::algorithm::is_any_of(","));

	for (size_t n = 0; n < fields.size(); n+=5)
	{
		Ptr<Node> node1 = Names::Find<Node> (fields[n]);
		NS_ASSERT_MSG (node1 != 0, fields[n] << "is not a Node");

		Ptr<L3Protocol> ndn1 = node1->GetObject<L3Protocol> ();
		NS_ASSERT_MSG (ndn1 != 0, "Ndn protocol hasn't been installed on a node, please install it first");

		shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace> (ndn1->getFaceById(atoi(fields[n+1].c_str())));
		//Ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace>(atoi(fields[n+1].c_str())));

		Ptr<Node> node2 = Names::Find<Node> (fields[n+2]);
		NS_ASSERT_MSG (node1 != 0, fields[n] << "is not a Node");

		cout << "\n 1: " << fields[n] << " 2: " <<  fields[n+1] << " 3: " << fields[n+2] << " 4: " << fields[n+3] << " 5: " << fields[n+4]<< endl;

	}

}


void
ControllerApp::CalculateRoutes()
{
  /**
   * Implementation of route calculation is heavily based on Boost Graph Library
   * See http://www.boost.org/doc/libs/1_49_0/libs/graph/doc/table_of_contents.html for more details
   */

  BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<boost::NdnControllerRouterGraph>));
  BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<boost::NdnControllerRouterGraph>));

  boost::NdnControllerRouterGraph graph;
  // typedef graph_traits < NdnControllerRouterGraph >::vertex_descriptor vertex_descriptor;

  // For now we doing Dijkstra for every node.  Can be replaced with Bellman-Ford or Floyd-Warshall.
  // Other algorithms should be faster, but they need additional EdgeListGraph concept provided by
  // the graph, which
  // is not obviously how implement in an efficient manner

  for (ns3::ndn::ControllerNodeList::Iterator node = ns3::ndn::ControllerNodeList::Begin (); node != ns3::ndn::ControllerNodeList::End (); node++)
  {
    Ptr<ControllerRouter> source = (*node);
    if (source == NULL) {
      NS_LOG_DEBUG("No node present in controller container");
      continue;
    }

    boost::DistancesMap distances;

    dijkstra_shortest_paths(graph, source,
                            // predecessor_map (boost::ref(predecessors))
                            // .
                            distance_map(boost::ref(distances))
                              .distance_inf(boost::WeightInf)
                              .distance_zero(boost::WeightZero)
                              .distance_compare(boost::WeightCompare())
                              .distance_combine(boost::WeightCombine()));

    // NS_LOG_DEBUG (predecessors.size () << ", " << distances.size ());

    //Ptr<L3Protocol> L3protocol = (*node)->GetObject<L3Protocol>();
    //shared_ptr<nfd::Forwarder> forwarder = L3protocol->getForwarder();

    std::cout << "\n Reachability from Node: " << source->GetSourceNode()<< std::endl;
    for (const auto& dist : distances) {
      if (dist.first == source)
        continue;
      else {
        // cout << "  Node " << dist.first->GetObject<Node> ()->GetId ();
        if (std::get<0>(dist.second) == 0) {
          // cout << " is unreachable" << endl;
        }
        else {
          std::list<std::tuple<shared_ptr<Name>,shared_ptr<Face>,size_t>> pathList;
          cout << "\n Destination node name - > " << dist.first->GetSourceNode() << endl;
          cout << "\n Size of prefix list " <<	dist.first->GetLocalPrefixes().size() << endl;
          for (const auto& prefix : dist.first->GetLocalPrefixes()) {
            cout << " prefix " << prefix->toUri().c_str() << " reachable via face " << std::get<0>(dist.second)->getId()
                 << " with distance " << std::get<1>(dist.second) << " with delay "
                 << std::get<2>(dist.second);
             pathList.push_back(std::make_tuple(prefix, std::get<0>(dist.second), std::get<1>(dist.second)));
            //FibHelper::AddRoute(*node, *prefix, std::get<0>(dist.second),
                                //std::get<1>(dist.second));
          }
          source->AddPaths(dist.first,pathList);
        }
      }
    }
  }
}


Ptr<ControllerRouter> ControllerApp::IsNodePresent(std::string strNodeName)
{
	Ptr<ControllerRouter> node = NULL;
	for (ns3::ndn::ControllerNodeList::Iterator j = ns3::ndn::ControllerNodeList::Begin (); j != ns3::ndn::ControllerNodeList::End (); j++)
	{
		if ((*j)->GetSourceNode().compare(strNodeName)==0)
		{
			node=(*j);
		}
	}
	return node;
}

bool ControllerApp::IsNodeActive(Ptr<ControllerRouter> node)
{
	for (ns3::ndn::ControllerNodeList::Iterator j = ns3::ndn::ControllerNodeList::Begin (); j != ns3::ndn::ControllerNodeList::End (); j++)
	{
		if((*j)==node)
		{
			return (*j)->GetStatus();
		}
	}
	return false;
}

std::string ControllerApp::getPrefix(Ptr<Node> NodeObj)
{
	std::string attrValue="";
	std::string strTempString;
	Ptr<AppPrefixHelper> appfxHelper = NodeObj->GetObject<AppPrefixHelper>();
	if (appfxHelper != 0) {
		std::map<TypeId, std::string> m_prefixmap = appfxHelper->GetMap();
		std::map<TypeId,std::string>::iterator itr;
		for (itr=m_prefixmap.begin(); itr!=m_prefixmap.end(); ++itr)
		{
			TypeId m_tid = itr->first;
			if(m_tid == this->GetTypeId())
			{
				if(!attrValue.empty())
				{
					attrValue.append(",");
				}
				attrValue.append(itr->second);
			}
		}
	  }

	/*
	Ptr<ndn::L3Protocol> l3 = NodeObj->GetObject<ndn::L3Protocol>();
	std::shared_ptr<ndn::nfd::Forwarder> fw = l3->getForwarder();
	ndn::nfd::Fib& fib = fw->getFib();
	for (const auto& fibEntry : fib) {
		strTempString = fibEntry.getPrefix().toUri().c_str();
		if(strTempString.compare("/localhost/nfd")!= 0)
		{
			if(!attrValue.empty())
			{
				attrValue.append(",");
			}
			attrValue.append(fibEntry.getPrefix().toUri().c_str());
		}
	}*/
	return attrValue;
}


std::string ControllerApp::GetLocalLinkInfo()
{
	std::stringstream strStateTemplate;
	//std::string strStateTemplate;
	bool firstVisit = false;

	Ptr<Node> localNode = GetNode ();

	Ptr<L3Protocol> ndn = localNode->GetObject<L3Protocol> ();
	NS_ASSERT_MSG (ndn != 0, "Ndn protocol hasn't been installed on a node, please install it first");

	NdnControllerString strControllerData = NdnControllerString("");
	strControllerData.SetSourceNode(Names::FindName(localNode).c_str());

	for (auto& faceId : ndn->getForwarder()->getFaceTable())
	    {
	      shared_ptr<NetDeviceFace> face = std::dynamic_pointer_cast<NetDeviceFace>(faceId);
	      if (face == 0)
		{
		  NS_LOG_DEBUG ("Skipping non-netdevice face");
		  continue;
		}

	      Ptr<NetDevice> nd = face->GetNetDevice ();
	      if (nd == 0)
		{
		  NS_LOG_DEBUG ("Not a NetDevice associated with NetDeviceFace");
		  continue;
		}

	      Ptr<Channel> ch = nd->GetChannel ();

	      if (ch == 0)
		{
		  NS_LOG_DEBUG ("Channel is not associated with NetDevice");
		  continue;
		}

	      if (ch->GetNDevices () == 2) // e.g., point-to-point channel
		{
		  for (uint32_t deviceId = 0; deviceId < ch->GetNDevices (); deviceId ++)
		    {
		      Ptr<NetDevice> otherSide = ch->GetDevice (deviceId);
		      if (nd == otherSide) continue;
		      Ptr<Node> otherNode = otherSide->GetNode ();
		      NS_ASSERT (otherNode != 0);
		      Ptr<L3Protocol> otherNdn = otherNode->GetObject<L3Protocol> ();
		      NS_ASSERT_MSG (otherNdn != 0, "Ndn protocol hasn't been installed on the other node, please install it first");
		      if(!firstVisit)
		      {
		    	  firstVisit=true;
		    	  strStateTemplate << Names::FindName(otherNode)  << "," << face->getId() << "," << face->getMetric();
		      }
		      else
		      {
		    	  strStateTemplate << ",";
		    	  strStateTemplate <<  Names::FindName(otherNode)  << "," << face->getId() << "," << face->getMetric();
		      }
		    }
		}
	 }

	strControllerData.SetLinkInfo(strStateTemplate.str());
	strControllerData.SetNodePrefixInfo(getPrefix(localNode));
	strControllerData.SetAppPrefixInfo("");

	return strControllerData.GetString();
}


void ControllerApp::AddControllerNodeInfo(Ptr<ControllerRouter> ControllerRouterNode)
{
	std::string strLinkInfo = GetLocalLinkInfo();
	NdnControllerString strControllerData = NdnControllerString(strLinkInfo);
	std::string strSourceNode ="";
	strSourceNode=strControllerData.GetSourceNode();
	std::vector<string> fields = strControllerData.GetLinkInfo();
	if (!fields.empty() and fields.size() >= 3)
	{
		for (size_t n = 0; n < fields.size(); n+=3)
			{
				Ptr<ControllerRouter> otherNode = IsNodePresent(fields[n]);
				if(otherNode == NULL)
				{
					Ptr<ControllerRouter> otherNode = CreateObject<ControllerRouter>(fields[n]);
					ns3::ndn::ControllerNodeList::Add(otherNode);
				}
				Ptr<Node> node1 = Names::Find<Node> (ControllerRouterNode->GetSourceNode());
				NS_ASSERT_MSG (node1 != 0, ControllerRouterNode->GetSourceNode() << "is not a Node");
				Ptr<L3Protocol> ndn1 = node1->GetObject<L3Protocol> ();
				NS_ASSERT_MSG (ndn1 != 0, "Ndn protocol hasn't been installed on a node, please install it first");
				shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace> (ndn1->getFaceById(atoi(fields[n+1].c_str())));
				ControllerRouterNode->AddIncidency(face, otherNode, atoi(fields[n+2].c_str()));
			}
	}
	AddPrefix(ControllerRouterNode, strControllerData.GetNodePrefixInfo());
}


void ControllerApp::AddIncidency(Ptr<ControllerRouter> node, std::vector<string> fields)
{
	if (!fields.empty() and fields.size() >= 3)
	{
		for (size_t n = 0; n < fields.size(); n+=3)
			{
				Ptr<ControllerRouter> otherNode = IsNodePresent(fields[n]);
				if(otherNode==NULL)
				{
					otherNode = CreateObject<ControllerRouter>(fields[n]);
					ns3::ndn::ControllerNodeList::Add(otherNode);
				}

				if ((otherNode->GetSourceNode().compare("Node1") == 0) and !otherNode->GetStatus())
				{
					AddControllerNodeInfo(otherNode);
				}

				Ptr<Node> node1 = Names::Find<Node> (node->GetSourceNode());
				NS_ASSERT_MSG (node1 != 0, node->GetSourceNode() << "is not a Node");

				Ptr<L3Protocol> ndn1 = node1->GetObject<L3Protocol> ();
				NS_ASSERT_MSG (ndn1 != 0, "Ndn protocol hasn't been installed on a node, please install it first");

				shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace> (ndn1->getFaceById(atoi(fields[n+1].c_str())));
				node->AddIncidency(face, otherNode, atoi(fields[n+2].c_str()));
			}
	}
}

void ControllerApp::AddPrefix(Ptr<ControllerRouter> node, std::vector<string> fields)
{
	if (!fields.empty())
	{
		for (size_t n = 0; n < fields.size(); n+=1)
			{
				auto name = make_shared<Name>(fields[n]);
				node->AddLocalPrefix(name);
			}
	}
}

void ControllerApp::sendInterestPacket(std::string strPrefix){
	shared_ptr<Name> nameWithSequence = make_shared<Name>(strPrefix);
	shared_ptr<Interest> interestConto = make_shared<Interest>();
	interestConto->setNonce(m_rand.GetValue());
	interestConto->setName(*nameWithSequence);
	time::milliseconds interestLifeTime(ndn::time::seconds(1000000));
	interestConto->setInterestLifetime(interestLifeTime);

	m_transmittedInterests(interestConto, this, m_face);
	m_face->onReceiveInterest(*interestConto);

	std::cout << "\n" << endl;
	std::cout << "\n";
}

std::string ControllerApp::getNodePathData(Ptr<ControllerRouter> dstNode)
{
	NdnControllerString strControllerData = NdnControllerString("");
	strControllerData.SetSourceNode(dstNode->GetSourceNode());
	std::string strPath = "";

	std::map<Ptr<ControllerRouter>,std::list<std::tuple<shared_ptr<Name>,shared_ptr<Face>,size_t>>>::const_iterator iter;
	for (iter = dstNode->GetPathInfo().begin();iter!=dstNode->GetPathInfo().end();iter++)
	{
		strPath.append("(");
		strPath.append(iter->first->GetSourceNode());
		strPath.append(":");
		int count = 0;
		int size = iter->second.size();

		std::list<std::tuple<shared_ptr<Name>,shared_ptr<Face>,size_t>>::const_iterator it;

		std::cout << "\n Size of list -> " << iter->second.size() << std::endl;

		for(it=iter->second.begin();it!=iter->second.end();it++);
		{


			//std::cout << std::get<0>(it) << std::endl;

			count++;
			//strPath.append(std::get<0>(it)->toUri().c_str());
			//strPath.append(",");
			//strPath.append(std::get<1>(it)->getId());
			//strPath.append(",");
			//strPath.append(std::get<2>(it));
			//if(count != size)
			//{
				//strPath.append(",");
			//}
		}
		strPath.append(")");
	}
	strControllerData.SetCalculatedPath(strPath);
	return strControllerData.GetString();
}


std::string ControllerApp::getTheCalculationPath(std::string strForNode){
	// We need to send following information to node in order to travel packet to shortest distance.
	// Node Id
	// Prefix
	// Face ID
	// Distance
	// Delay

	std::string strPath="";
	Ptr<ControllerRouter> dstNode = IsNodePresent(strForNode);
	if(dstNode!=NULL)
	{
		strPath = getNodePathData(dstNode);
		//strPath = "Node Name -> " + strForNode + "," + "\t Prefix Name -> " + "/controller" + "," + "\t Face ID -> " + "256" + "," + "\t Face Metrics (Link weight) -> " + "3" + "," + "\t Face delay-> " + "0";
		strPath = strForNode + "," + "/controller" + "," + "256" + "," + "3" + "," + "0";
	}
	return strPath;
}



void ControllerApp::sendPathDataPacket(std::shared_ptr<const Interest> interest){

	if (!m_active)
		return;
	NS_LOG_FUNCTION_NOARGS ();
	NS_LOG_FUNCTION(this << interest);

	Name dataName(interest->getName());
	auto dPacket = make_shared<Data>();
	dPacket->setName(dataName);
	dPacket->setFreshnessPeriod(ndn::time::milliseconds(6000));
	std::string strTemplateNode = getTheCalculationPath(extractNodeName(interest->getName().toUri(), 2));
	dPacket->setContent(reinterpret_cast<const uint8_t*>(strTemplateNode.c_str()), (uint32_t) strTemplateNode.length());

	Signature signature;
	SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));
	if (m_keyLocator.size() > 0) {
		signatureInfo.setKeyLocator(m_keyLocator);
	}
	signature.setInfo(signatureInfo);
	signature.setValue(Block(&m_signature, sizeof(m_signature)));

	dPacket->setSignature(signature);
	dPacket->wireEncode();

	std::cout << "\n CustConsumerApp: Data packet- > " << dPacket->getName () << " is sending from face -> " << m_face << std::endl;
	m_transmittedDatas(dPacket, this, m_face);
	m_face->onReceiveData(*dPacket);
	std::cout << "\n";
}


void ControllerApp::OnInterest(std::shared_ptr<const Interest> interest) {
	App::OnInterest(interest); // tracing inside
	NS_LOG_FUNCTION(this << interest);
	std::cout << "\n CentralizedControllerApp: Received interest packet -> " << interest->getName() << std::endl;
	if (!m_active)
		return;

	std::string strPrefix;
	std::string strRequestType = extractNodeRequestType(interest->getName().toUri());
	std::string strInterestNodePrefix = extractNodeName(interest->getName().toUri(), 2);

	if (strRequestType.compare("req_route") == 0)
	{
		strPrefix = "/" + strInterestNodePrefix + "/controller" + "/req_route";
		std::cout << "\n CentralizedControllerApp: Sending interest packet to  " << strInterestNodePrefix << "  with acknowledge interest -> " << strPrefix << std::endl;
		sendInterestPacket(strPrefix);
	}
	else if(strRequestType.compare("res_route") == 0)
	{
		std::cout << "\n CentralizedControllerApp: Sending data packet to  " << strInterestNodePrefix << "  with calculated distance "<< std::endl;
		strPrefix = "/" + strInterestNodePrefix + "/controller" + "/res_route";
		//sendPathDataPacket(interest);
	}
	else{
		strPrefix = "/";
		sendInterestPacket(strPrefix);
	}

}

void ControllerApp::StartSendingPathToNode()
{

	std::cout << "\n ******* ****************************** Starting Controller to Consumer Communication ************************************************************"<<std::endl;
	//std::string strInterestPrefix = "/" + extractNodeName(contentObject->getName().toUri(), 1) + "/controller" + "/res_route";
	//std::cout << "\n CentralizedControllerApp: Sending interest packet to  " << strInterestPrefix << std::endl;

	for (ns3::ndn::ControllerNodeList::Iterator node = ns3::ndn::ControllerNodeList::Begin (); node != ns3::ndn::ControllerNodeList::End (); node++)
	  {
		Ptr<ControllerRouter> source = (*node);
		if (source != NULL){
			std::string strInterestPrefix = "/" + source->GetSourceNode() + "/controller" + "/res_route";
			std::cout << "\n ControllerApp: Sending interest packet to  " << strInterestPrefix << std::endl;
			sendInterestPacket(strInterestPrefix);
		}
	  }
}


void ControllerApp::OnData(std::shared_ptr<const Data> contentObject) {
	App::OnData(contentObject); // tracing inside
	NS_LOG_FUNCTION(this << contentObject);
	if (!m_active)
    	return;
	NS_LOG_INFO ("\n Received content object: " << boost::cref(*contentObject));
	std::cout << "\n CentralizedControllerApp: Received Data packet -> "
			<< contentObject->getName() << std::endl;
	std::string msg(reinterpret_cast<const char*>(contentObject->getContent().value()),
			contentObject->getContent().value_size());

	NdnControllerString strControllerData = NdnControllerString(msg);
	std::string strSourceNode =	strControllerData.GetSourceNode();

	cout << "\n Source Node -> " << strSourceNode << endl;
	cout << "\n msg -> " << msg << endl;

	Ptr<ControllerRouter> node = IsNodePresent(strSourceNode);

	if(node==NULL)
	{
		node = CreateObject<ControllerRouter>(strSourceNode);
		size_t k = ns3::ndn::ControllerNodeList::Add(node);
	}

	AddIncidency(node, strControllerData.GetLinkInfo());
	AddPrefix(node, strControllerData.GetNodePrefixInfo());

	if(strSourceNode.compare("Node3") == 0)
	{
		CalculateRoutes();
		//StartSendingPathToNode();
	}
}

void ControllerApp::OnNack(std::shared_ptr<const ndn::Interest> interest) {
	//App::OnNack(interest); // tracing inside
	NS_LOG_FUNCTION(this << interest);
	std::cout << "\n";
	std::cout << "CentralizedControllerApp: Received Nack packet -> "
			<< interest->getName() << std::endl;
}

} // namespace ndn
}
// namespace ns3
