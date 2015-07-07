#include "ndn-controllerapp.hpp"
#include "ns3/log.h"

//#include "ns3/ndn-interest.hpp"
//#include "ns3/ndn-data.hpp"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"

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

#include "boost/tuple/tuple.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/concept/assert.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include "helper/boost-graph-ndn-controller-routing-helper.hpp"
#include "helper/controller-node-list.hpp"
#include "model/ndn-yanGraphAlgorithm.hpp"
#include "helper/ndn-app-prefix-helper.hpp"

#include <boost/ref.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <typeinfo>

namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE("ndn.ControllerApp");

using namespace std;
using namespace nfd;

namespace ns3 {
namespace ndn {


const std::string ControllerApp::INFO_COMPONENT = "INFO";
const std::string ControllerApp::HELLO_COMPONENT = "HELLO";

NS_OBJECT_ENSURE_REGISTERED(ControllerApp);

int ControllerApp::counter = 0;

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

	std::cout<< "####################################### Controller Initialization ###############################################################"<< std::endl;
	std::cout << "\n";
	initialize();

}

void
ControllerApp::StopApplication() {
	NS_LOG_FUNCTION_NOARGS ();
	App::StopApplication();
}

void
ControllerApp::initialize()
{
	// Get all neighbor information
	m_gb_adList = CollectLinks();
	scheduleHelloPacketEvent(30);
	schedulecheckLinkEvent(60);
}

void ControllerApp::scheduleHelloPacketEvent(uint32_t seconds)
{
	m_helloEvent = scheduler::schedule(ndn::time::seconds(seconds),bind(&ControllerApp::sendScheduledHelloInterest, this, seconds));
}

void ControllerApp::schedulecheckLinkEvent(uint32_t seconds)
{
	m_checkEvent = scheduler::schedule(ndn::time::seconds(seconds),bind(&ControllerApp::VerifyLinks, this, seconds));
}

void
ControllerApp::scheduleFailEvent(uint32_t seconds)
{
	cout <<"\n Called scheduleFailEvent" <<endl;
	m_failEvent = scheduler::schedule(ndn::time::seconds(seconds),bind(&ControllerApp::SchedulerHandlingFailureCalc, this, seconds));
	//Simulator::Schedule(Seconds(10.0), ndn::LinkControlHelper::FailLink, node1, node2);
}


void
ControllerApp::OnTimeout(uint32_t sequenceNumber)
{
	cout << "\n OnTimeout Called " << endl;
}

void
ControllerApp::expressInterest(const Name& interestName, uint32_t seconds)
{
	cout<< "\n Expressing Hello Interest from controller :" << interestName << endl;
  	//Interest i(interestName);
  	//i.setInterestLifetime(ndn::time::seconds(seconds));
  	//i.setMustBeFresh(true);

  	shared_ptr<Interest> interestConto = make_shared<Interest>();
    UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
  	interestConto->setNonce(m_rand.GetValue());
  	interestConto->setName(interestName);
  	time::milliseconds interestLifeTime(ndn::time::seconds(1000000));
  	interestConto->setInterestLifetime(interestLifeTime);
  	//interestConto->setMustBeFresh(true);
  	m_transmittedInterests(interestConto, this, m_face);
  	m_face->onReceiveInterest(*interestConto);
  	std::cout << "\n";
  //m_face.expressInterest(i,ndn::bind(&HelloProtocol::onContent,this,_1, _2),ndn::bind(&HelloProtocol::processInterestTimedOut,
   //                                           this, _1));

}

void ControllerApp::sendScheduledHelloInterest(uint32_t seconds)
{
	counter++;
	cout <<"\n Called Controller sendScheduledHelloInterest function ------- " <<endl;
	cout << "\n Source node for Hello packets->  " << Names::FindName(GetNode())<<endl;
	std::list<Adjacent> adjList = m_gb_adList.getAdjList();
	for (std::list<Adjacent>::iterator it = adjList.begin(); it != adjList.end();
	       ++it) {
	    if((*it).getFaceId() != 0) {
	      /* interest name: /<neighbor>/HELLO/INFO/ */
	      Name interestName = (*it).getName() ;
	      interestName.append(Names::FindName(GetNode()));
	      interestName.append(HELLO_COMPONENT);
	      interestName.append(INFO_COMPONENT);
	      m_gb_adList.incrementInterestSendCount((*it).getName());
	      expressInterest(interestName,m_conf.getInterestResendTime());
	    }
	  }

	if(!m_helloEvent->IsRunning())
	{
		cout <<"\n This movement event is not running " << endl;
		//scheduleHelloPacketEvent(m_conf.getInfoInterestInterval());
	}

	if(m_helloEvent->IsExpired())
	{
		cout <<"\n This movement event is expired " << endl;
		//cout << "\n Printing list value after sending Hello packets " <<endl;
		//m_gb_adList.writeLog();
	}
}

AdjacencyList ControllerApp::CollectLinks()
{
	AdjacencyList objAList;
	Ptr<Node> localNode = GetNode ();
	Ptr<L3Protocol> ndn = localNode->GetObject<L3Protocol> ();
	NS_ASSERT_MSG (ndn != 0, "Ndn protocol hasn't been installed on a node, please install it first");
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
	    		  Adjacent objAdjacent;
	    		  Ptr<NetDevice> otherSide = ch->GetDevice (deviceId);
	    		  if (nd == otherSide)
	    			  continue;
	    		  Ptr<Node> otherNode = otherSide->GetNode ();
	    		  NS_ASSERT (otherNode != 0);
	    		  Ptr<L3Protocol> otherNdn = otherNode->GetObject<L3Protocol> ();
	    		  NS_ASSERT_MSG (otherNdn != 0, "Ndn protocol hasn't been installed on the other node, please install it first");
	    		  objAdjacent.setConnectedNode(otherNode);
	    		  objAdjacent.setName(Names::FindName(otherNode));
	    		  objAdjacent.setFaceId(face->getId());
	    		  //objAdjacent.setConnectingFaceUri(face->getRemoteUri().c_str());
	    		  objAdjacent.setLinkCost(face->getMetric());
	    		  if(face->isUp())
	    		  {
	    			  objAdjacent.setStatus(Adjacent::STATUS_ACTIVE);
	    		  }
	    		  else if(!face->isUp())
	    		  {
	    			  objAdjacent.setStatus(Adjacent::STATUS_INACTIVE);
	    		  }
	    		  else
	    		  {
	    			  objAdjacent.setStatus(Adjacent::STATUS_UNKNOWN);
	    		  }
	    		  objAdjacent.setInterestSentNo(0);
	    		  objAdjacent.setDataRcvNo(0);
	    		  objAList.insert(objAdjacent);
		    }
		}
	 }
	return objAList;
}


std::string ControllerApp::extractNodeName(std::string strPacketName, int n) {

	std::cout << strPacketName;
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strPacketName,
			boost::algorithm::is_any_of("/"));
	return fields[n];
}

/*
std::string ControllerApp::extractNodeRequestType(std::string strPrefixName) {
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strPrefixName,
			boost::algorithm::is_any_of("/"));
	return fields[3];
}
*/

std::string ControllerApp::extractNodeRequestType(std::string strPrefixName, int index) {
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strPrefixName,
			boost::algorithm::is_any_of("/"));

	//for (size_t n = 0; n < fields.size(); n++)
	//	std::cout << fields[n] << "\"\n";
	//cout << endl;

	if(index <= fields.size())
	{
		return fields[index];
	}
	else
	{
		return fields[0];
	}

	/*

	if (fields.size()>=3)
	{
		return fields[3];
	}
	else
	{
		return fields[0];
	}
	*/
}

void ControllerApp::VerifyLinks(uint32_t seconds)
{
	// For this algorithm, we will have to cover following cases
	// 1: Both list size, if yes compare each object with other
	// 2: if, not, then if current list size greater than the old one, that case new object should send to controller, and rest of the objects will compare each other
	// 3: if, current list is smaller than old one, then old's one object no more present as link, so need to send that object info to controller as link_removed
	cout <<"\n VeryfyLinks: Called for node -> " << Names::FindName(GetNode()).c_str()<<endl;
	cout <<"\n Printing global list values ->  " << endl;
	m_gb_adList.writeLog();

	bool isReqtoController=false;
	std::stringstream strUpdateToController;
	int sizeList=0;

	AdjacencyList local_adList = CollectLinks();
	isReqtoController = m_gb_adList.isAdjBuildable();

	std::list<Adjacent>& adjList1 = local_adList.getAdjList();
	std::list<Adjacent>& adjList2 = m_gb_adList.getAdjList();

	if (m_gb_adList.getSize() > local_adList.getSize()) {
		sizeList=1;
	} else if(m_gb_adList.getSize() < local_adList.getSize()) {
		sizeList=2;
	} else{
		sizeList=0;
	}

	std::list<Adjacent>::iterator it1;
	if( sizeList==2 || sizeList==0 )
	{
		for (it1=adjList1.begin();it1!= adjList1.end();it1++)
		{
			Adjacent *adj = m_gb_adList.findAdjacent(it1->getName());
			if(adj!=NULL)
			{
				// compare each value with other one along with interest and data
				if(((*it1) == (*adj)))
				{
					if(adj->getInterestSentNo()>0 && adj->getDataRcvNo()==0)
					{
						// It means other NFD is not able to send the information
						strUpdateToController << adj->getName()  << "," << adj->getFaceId() << "," << adj->getLinkCost() << "," << adj->getStatus() << "," << "FACE_DOWN";
					}
				}
				else
				{
					// send this (*it) information to controller.
					strUpdateToController << it1->getName()  << "," << it1->getFaceId() << "," << it1->getLinkCost() << "," << it1->getStatus();
					if((it1->getStatus() - adj->getStatus())!=0)
					{
						if(it1->getStatus()==Adjacent::STATUS_ACTIVE)
						{
							strUpdateToController << "," << "FACE_UP";
						}

						if(it1->getStatus()==Adjacent::STATUS_INACTIVE)
						{
							strUpdateToController << "," << "FACE_DOWN";
						}
						continue;
					}

					if((it1->getLinkCost() != adj->getLinkCost()))
					{
						strUpdateToController << "," << "LINK_COST";
						continue;
					}
				}

			}
			else
			{
				// This object has been added new in the current link.
				// send this (*it) information to controller
				strUpdateToController << adj->getName()  << "," << adj->getFaceId() << "," << adj->getLinkCost() << "," << adj->getStatus() << "," << "LINK_ADDED";
			}

			if(!strUpdateToController.str().empty())
			{
				strUpdateToController << ",";
			}
		}
	}
	else
	{
		for (it1=adjList2.begin();it1!= adjList2.end();it1++)
		{
					Adjacent *adj = local_adList.findAdjacent(it1->getName());
					if(&adj!=NULL)
					{
						// compare each value with other one along with interest and data
						if(adj == &(*it1))
						{
							if(it1->getInterestSentNo()>0 && it1->getDataRcvNo()==0)
							{
								// It means other NFD is not able to send the information
								strUpdateToController << it1->getName()  << "," << it1->getFaceId() << "," << it1->getLinkCost() << "," << it1->getStatus() << "," << "FACE_DOWN";
							}
						}
						else
						{
							// send this (*it) information to controller.
							strUpdateToController << adj->getName()  << "," << adj->getFaceId() << "," << adj->getLinkCost() << "," << adj->getStatus();
							if((adj->getStatus() - it1->getStatus())!=0)
							{
								if(adj->getStatus()==Adjacent::STATUS_ACTIVE)
								{
									strUpdateToController << "," << "FACE_UP";
								}

								if(adj->getStatus()==Adjacent::STATUS_INACTIVE)
								{
									strUpdateToController << "," << "FACE_DOWN";
								}
								continue;
							}

							if((adj->getLinkCost() != it1->getLinkCost()))
							{
								strUpdateToController << "," << "LINK_COST";
								continue;
							}
						}

					}
					else
					{
						// This object has been added new in the current link.
						// send this (*it) information to controller
						strUpdateToController << it1->getName()  << "," << it1->getFaceId() << "," << it1->getLinkCost() << "," << it1->getStatus() << "," << "LINK_REMOVED";
					}

					if(!strUpdateToController.str().empty())
					{
						strUpdateToController << ",";
					}
		}

	}

	if(isReqtoController && strUpdateToController==NULL)
	{
		// mismatch occure somthing wrong in code
		std::cout << "\n Mismatch occure in global list and local list comaparison, need to handle this" << std::endl;
	}

	if(!strUpdateToController.str().empty())
	{
		//ControllerSync(strUpdateToController);
	}
	m_gb_adList.reset();
	m_gb_adList.setAdjList(adjList1);
}

void ControllerApp::ControllerSync(std::stringstream& strUpdateToController)
{
	// Check first in FIB if current prefix has available in case if prefix has down or cost increased.
	// Prepare string for controller send.

	cout <<"\n ControllerSync: Called " << endl;
	cout <<"\n Printing values for controller -> " << strUpdateToController.str() <<endl;
	std::stringstream strmodifiedControllerData;
	NdnControllerString strControllerData = NdnControllerString("");
	if(strUpdateToController!=NULL)
	{
		strControllerData.SetSourceNode(Names::FindName(GetNode()).c_str());
		strControllerData.SetLinkUpdateInfo(strUpdateToController.str());
		std::vector<string> fields = strControllerData.GetLinkUpdateInfo();

		if (!fields.empty() and fields.size() >= 5)
		{
			Ptr<Node> localNode = GetNode();
			Ptr<ndn::L3Protocol> l3 = localNode->GetObject<ndn::L3Protocol>();
			std::shared_ptr<ndn::nfd::Forwarder> fw = l3->getForwarder();
			ndn::nfd::Fib& fib = fw->getFib();
			for (size_t n = 0; n < fields.size(); n+=5)
			{
				int NextHopcounter=0;
				for (const auto& fibEntry : fib)
				{
					std::string strTempString = fibEntry.getPrefix().toUri().c_str();
					if(strTempString.compare(fields[n]) == 0)
					{
						for (const auto& nh : fibEntry.getNextHops())
						{
							if(nh.getFace()->getId()!=l3->getFaceById(atoi(fields[n+1].c_str()))->getId())
							{
								NextHopcounter++;
							}
							//std::cout << "  - " << nh.getFace() << ", " << nh.getFace()->getId() << ", " << nh.getCost() << std::endl;
						}
					}
				}
				if(NextHopcounter==0)
				{
					strmodifiedControllerData << fields[n]  << "," << fields[n+1] << "," << fields[n+2] << "," << fields[n+3] << "," << fields[n+4];
					if(n <= fields.size())
					{
						strmodifiedControllerData << ",";
					}
				}
			}
		}
	}

	if(strmodifiedControllerData!=NULL)
	{
		m_strUpdateToController << strmodifiedControllerData;
		std::string strControllerUpdate = "/controller/" + Names::FindName(Ptr<Node>(GetNode())) + "/req_update";
		sendInterestPacket(strControllerUpdate);
	}

}

void ControllerApp::SendUpdateDataPacketToController(shared_ptr<const Interest> interest) {

	if (!m_active)
		return;
	NS_LOG_FUNCTION_NOARGS ();
	NS_LOG_FUNCTION(this << interest);

	std::cout<< "CustConsumerApp: Sending local changed into data Packet to controller-> "<< interest->getName() << std::endl;
	Name dataName(interest->getName());
	auto dPacket = make_shared<Data>();
	dPacket->setName(dataName);
	//dPacket->setFreshnessPeriod(ndn::time::milliseconds(6000));
	dPacket->setFreshnessPeriod(ndn::time::milliseconds(20000));


	NdnControllerString strControllerData = NdnControllerString("");
	strControllerData.SetSourceNode(Names::FindName(GetNode()).c_str());
	strControllerData.SetLinkUpdateInfo(m_strUpdateToController.str());
	std::string strTemplateNode=strControllerData.GetString();
	dPacket->setContent(reinterpret_cast<const uint8_t*>(strTemplateNode.c_str()), (uint32_t) strTemplateNode.length());
	//ndn::StackHelper::getKeyChain().sign(*dPacket);
	Signature signature;
	SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));
	if (m_keyLocator.size() > 0){
		signatureInfo.setKeyLocator(m_keyLocator);
	}
	signature.setInfo(signatureInfo);
	signature.setValue(Block(&m_signature, sizeof(m_signature)));

	dPacket->setSignature(signature);
	dPacket->wireEncode();

	std::cout << "\n CustConsumerApp: Data packet- > " << dPacket->getName () << " is sending from face -> " << m_face->getId() << std::endl;
	m_transmittedDatas(dPacket, this, m_face);
	m_face->onReceiveData(*dPacket);
	std::cout << "\n";

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

void ControllerApp::LinkInitalization(Ptr<ControllerRouter> source, shared_ptr<Face> faceId, Ptr<ControllerRouter> dest)
{
	cout << "\n LinkInitalization -------> Start -> "<<endl;
	for (ns3::ndn::ControllerNodeList::Iterator j = ns3::ndn::ControllerNodeList::Begin (); j != ns3::ndn::ControllerNodeList::End (); j++)
		  {
			  	  if((*j)->GetId() != 0)
			  	  {
			  		  	  if ((*j)!= source)
			  		  	  {
			  		  		cout << "\n LinkInitalization -> Scanning Node -> "<< (*j)->GetSourceNode()<<endl;
			  		  		(*j)->ResetMultiPathIncidencies();
			  		  		std::list<ndn::ControllerRouter::Incidency> lstForLinkDown = (*j)->GetIncidencies();
			  		  		std::list<ndn::ControllerRouter::Incidency>::iterator checkItemDown;
			  		  		for (checkItemDown=lstForLinkDown.begin();checkItemDown!=lstForLinkDown.end();checkItemDown++)
			  		  		 {
			  		  			if (std::get<0>(*checkItemDown)==dest ||  std::get<2>(*checkItemDown)!=source)
			  		  			{
			  		  				cout << "\n LinkInitalization -> Adding value -> "<< std::get<1>(*checkItemDown)->getId()<<endl;
			  		  				cout << "\n LinkInitalization -> Adding value -> "<< std::get<2>(*checkItemDown)->GetSourceNode() <<endl;
			  		  				cout << "\n LinkInitalization -> Adding value -> "<< std::get<3>(*checkItemDown) <<endl;
			  		  				(*j)->AddMultiPathIncidency(std::get<1>(*checkItemDown),std::get<2>(*checkItemDown),std::get<3>(*checkItemDown));
			  		  			}
			  		  		 }
			  		  	  }
			  	  }
		  }
	cout << "\n LinkInitalization -------> Stop -> "<<endl;
}


void
ControllerApp::CalculateRoutesSinglePath()
{
  /**
   * Implementation of route calculation is heavily based on Boost Graph Library
   * See http://www.boost.org/doc/libs/1_49_0/libs/graph/doc/table_of_contents.html for more details
   */

    typedef std::list<tuple<std::shared_ptr<Name>,std::shared_ptr<Face>,size_t>> pathList;
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
    std::cout << "\n *************************************************************************" <<std::endl;
    std::cout << "\n Single path calculation for source node Name -> " << source->GetSourceNode() <<std::endl;
    //std::list<ndn::ControllerRouter::Incidency> lstAllIncidencies = source->GetIncidencies();
    //std::list<ndn::ControllerRouter::Incidency>::iterator listitem;

    //for (listitem=lstAllIncidencies.begin();listitem!=lstAllIncidencies.end();listitem++)
    //{
    	//std::cout << "\n Calulating Paths from Face of the source node ->  " << std::get<1>(*listitem)->getId()<< std::endl;
    	//source->ResetMultiPathIncidencies();
    	//source->AddMultiPathIncidency(std::get<1>(*listitem),std::get<2>(*listitem),std::get<3>(*listitem));
    	//LinkInitalization(source,std::get<1>(*listitem),std::get<2>(*listitem));

        BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<boost::NdnControllerRouterGraph>));
        BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<boost::NdnControllerRouterGraph>));

        boost::NdnControllerRouterGraph graph;
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

    	std::cout << "\n Reachability from Node: " << source->GetSourceNode() << std::endl;
    	for (const auto& dist : distances) {
    		if (dist.first == source)
    			continue;
    		else {
    			// cout << "  Node " << dist.first->GetObject<Node> ()->GetId ();
    			//source->ResetPaths();
    			if (std::get<0>(dist.second) == 0) {
    				// cout << " is unreachable" << endl;
    			}
    			else {
    				pathList t1;
    				cout << "\n Destination node name - > " << dist.first->GetSourceNode() << endl;
    				cout << "\n Size of prefix list " <<	dist.first->GetLocalPrefixes().size() << endl;
    				for (auto& prefix : dist.first->GetLocalPrefixes()) {
    					cout << "prefix " << prefix->toUri().c_str() << " reachable via face " << std::get<0>(dist.second)->getId()
                		 << " with distance " << std::get<1>(dist.second) << " with delay "
                		 << std::get<2>(dist.second);
    					t1.push_back(std::make_tuple (prefix, std::get<0>(dist.second), std::get<1>(dist.second)));
    					//FibHelper::AddRoute(*node, *prefix, std::get<0>(dist.second),
                                	//std::get<1>(dist.second));
    				}
    				source->AddPaths(dist.first,t1);
    			}
    		}
    	}
    	std::cout << "\n End Single path calculation for source Node Name -> " << source->GetSourceNode() << std::endl;
    }
  //}
  std::cout<<"\n *************************************************************************"<<std::endl;
}



void
ControllerApp::CalculateRoutes()
{
  /**
   * Implementation of route calculation is heavily based on Boost Graph Library
   * See http://www.boost.org/doc/libs/1_49_0/libs/graph/doc/table_of_contents.html for more details
   */

    typedef std::list<tuple<std::shared_ptr<Name>,std::shared_ptr<Face>,size_t>> pathList;
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

    std::cout << "\n *************************************************************************" <<std::endl;
    std::cout << "\n MultiPath calculation for Source Node Name -> " << source->GetSourceNode() <<std::endl;

    std::list<ndn::ControllerRouter::Incidency> lstAllIncidencies = source->GetIncidencies();
    std::list<ndn::ControllerRouter::Incidency>::iterator listitem;

    for (listitem=lstAllIncidencies.begin();listitem!=lstAllIncidencies.end();listitem++)
    {
    	std::cout << "\n Calulating Paths from Face of the source node ->  " << std::get<1>(*listitem)->getId()<< std::endl;
    	source->ResetMultiPathIncidencies();
    	source->AddMultiPathIncidency(std::get<1>(*listitem),std::get<2>(*listitem),std::get<3>(*listitem));
    	LinkInitalization(source,std::get<1>(*listitem),std::get<2>(*listitem));

        BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<boost::NdnControllerRouterGraph>));
        BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<boost::NdnControllerRouterGraph>));

        boost::NdnControllerRouterGraph graph;
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

    	std::cout << "\n Reachability from Node: " << source->GetSourceNode() << std::endl;
    	for (const auto& dist : distances) {
    		if (dist.first == source)
    			continue;
    		else {
    			// cout << "  Node " << dist.first->GetObject<Node> ()->GetId ();
    			//source->ResetPaths();
    			if (std::get<0>(dist.second) == 0) {
    				// cout << " is unreachable" << endl;
    			}
    			else {
    				pathList t1;
    				cout << "\n Destination node name - > " << dist.first->GetSourceNode() << endl;
    				cout << "\n Size of prefix list " <<	dist.first->GetLocalPrefixes().size() << endl;
    				for (auto& prefix : dist.first->GetLocalPrefixes()) {
    					cout << "prefix " << prefix->toUri().c_str() << " reachable via face " << std::get<0>(dist.second)->getId()
                		 << " with distance " << std::get<1>(dist.second) << " with delay "
                		 << std::get<2>(dist.second);
    					t1.push_back(std::make_tuple (prefix, std::get<0>(dist.second), std::get<1>(dist.second)));
    					//FibHelper::AddRoute(*node, *prefix, std::get<0>(dist.second),
                                	//std::get<1>(dist.second));
    				}
    				source->AddPaths(dist.first,t1);
    			}
    		}
    	}
    	std::cout << "\n Done Calulating Paths from Face of the source node ->  " << std::get<1>(*listitem)->getId()<< std::endl;
    }

    source->ResetMultiPathIncidencies();
    std::cout << "\n *************************************************************************" <<std::endl;
    std::cout << "\n End MultiPath calculation for Source Node Name -> " << source->GetSourceNode() <<std::endl;
  }
}


void
ControllerApp::initCalculationKPath(){
	my_graph.clear();
	int ver_no = ControllerNodeList::GetNNodes();
	if(ver_no > 2)
	{
		my_graph.setVertexNo(ver_no);
		std::cout <<"\n Initalizing Yan's k path"<<std::endl;
		for(ns3::ndn::ControllerNodeList::Iterator node = ns3::ndn::ControllerNodeList::Begin (); node != ns3::ndn::ControllerNodeList::End (); node++)
		 {
			std::cout <<"\n Source node is -> "<<(*node)->GetSourceNode()<<std::endl;
			std::list<std::tuple<Ptr<ControllerRouter>, shared_ptr<Face>, Ptr<ControllerRouter>, size_t>> adjancyList = (*node)->GetIncidencies();
			std::list<std::tuple<Ptr<ControllerRouter>, shared_ptr<Face>, Ptr<ControllerRouter>, size_t>>::iterator iter;
			for (iter = adjancyList.begin();iter!=adjancyList.end();iter++){
				Ptr<ControllerRouter> dstNode = std::get<2>(*iter);
				std::cout <<"\n Destination node is -> "<<dstNode->GetSourceNode()<<std::endl;
				std::cout <<"\n Face is -> "<<std::get<1>(*iter)->getId()<<std::endl;
				int cost = std::get<3>(*iter);
				std::cout <<"\n Cost is -> "<<cost<<std::endl;
				my_graph.add_incidency((*node),dstNode,cost);
			}
		 }
	}
}


std::shared_ptr<nfd::Face> ControllerApp::GetFaceId(Ptr<ControllerRouter> srcNode, Ptr<ControllerRouter> dstNode)
{
		if(srcNode!=NULL && dstNode!=NULL)
		{
			std::list<std::tuple<Ptr<ControllerRouter>, shared_ptr<Face>, Ptr<ControllerRouter>, size_t>> adjancyList = srcNode->GetIncidencies();
			std::list<std::tuple<Ptr<ControllerRouter>, shared_ptr<Face>, Ptr<ControllerRouter>, size_t>>::iterator iter;
			for (iter = adjancyList.begin();iter!=adjancyList.end();iter++){
				if(dstNode == std::get<2>(*iter))
				{
					return std::get<1>(*iter);
				}
			}
		}
		return NULL;
}

void
ControllerApp::CalculateKPathYanAlgorithm(int kpath){
	initCalculationKPath(); // Initializing Yan's K path data structures.
	cout <<"\n Calculating Yan'k path algorithm ---" <<endl;
	my_graph.printVertexInfo();
	cout <<"\n Total nodes in the grpah are -> "<< ControllerNodeList::GetNNodes() << endl;

	clock_t begin = clock();
	cout << "\nCalculating K shortest path Start time ->  "<< (double)begin/CLOCKS_PER_SEC;
	if(ControllerNodeList::GetNNodes()>2)
	{
		  typedef std::list<tuple<std::shared_ptr<Name>,std::shared_ptr<Face>,size_t>> pathList;
		  for (ns3::ndn::ControllerNodeList::Iterator src = ns3::ndn::ControllerNodeList::Begin (); src != ns3::ndn::ControllerNodeList::End (); src++)
		  {
			cout <<"\n Source node Name -> " << (*src)->GetSourceNode() << endl;
			for (ns3::ndn::ControllerNodeList::Iterator dst = ns3::ndn::ControllerNodeList::Begin (); dst != ns3::ndn::ControllerNodeList::End (); dst++)
				  {
						if((*src)!=(*dst))
						{
							cout << "\n ----------- Start K PATH algorithm for destination  "<<(*dst)->GetSourceNode()<< "----------";
							YenTopKShortestPathsAlg yenAlg(my_graph, my_graph.get_vertex(*src), my_graph.get_vertex(*dst));
							int i=1;
							if(kpath>0)
							{
								vector<BasePath*> result_list;

								pathList t1;
								yenAlg.get_shortest_paths(my_graph.get_vertex(*src),my_graph.get_vertex(*dst),kpath,result_list);
								cout <<"\n";
								for(vector<BasePath*>::const_iterator pos=result_list.begin();
										pos!=result_list.end(); ++pos)
								{
									cout <<"Path no " << i << endl;
									(*pos)->PrintOut(cout);

									Ptr<ControllerRouter> nextNode = (*pos)->getNextNode();
									if(nextNode!=NULL)
									{
										std::shared_ptr<nfd::Face> faceId = GetFaceId((*src),nextNode);
										if(faceId!=NULL)
										{
											for (auto& prefix : (*dst)->GetLocalPrefixes())
											{
												cout << "prefix " << prefix->toUri().c_str() << " reachable via face " << faceId->getId()
													 << " with distance " << (*pos)->Weight();
											    t1.push_back(std::make_tuple (prefix,faceId,static_cast<size_t>((*pos)->Weight())));
											}
										}
									}
									i++;
								}
								(*src)->AddPaths((*dst),t1);
								cout << "\n ----------- Stop K PATH algorithm for destination  "<<(*dst)->GetSourceNode()<< "----------";
							}
							else
							{
								while(yenAlg.has_next())
								{
									++i;
									yenAlg.next()->PrintOut(cout);
								}
							}
						}
				 }
		  }
	}

	clock_t end = clock();
	cout << "\nCalculating K shortest path End time ->  "<< (double)end/CLOCKS_PER_SEC;
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout << "\nIt took Yan's k path for all nodes" << elapsed_secs << "(seconds)" <<endl;

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
					otherNode = CreateObject<ControllerRouter>(fields[n]);
					ns3::ndn::ControllerNodeList::Add(otherNode);
				}
				Ptr<Node> node1 = Names::Find<Node> (ControllerRouterNode->GetSourceNode());
				NS_ASSERT_MSG (node1 != 0, ControllerRouterNode->GetSourceNode() << "is not a Node");
				Ptr<L3Protocol> ndn1 = node1->GetObject<L3Protocol> ();
				NS_ASSERT_MSG (ndn1 != 0, "Ndn protocol hasn't been installed on a node, please install it first");
				shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace> (ndn1->getFaceById(atoi(fields[n+1].c_str())));
				ControllerRouterNode->AddIncidency(face, otherNode, atoi(fields[n+2].c_str()));
				//my_graph.add_incidency(ControllerRouterNode,otherNode,atof(fields[n+2].c_str()));

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
				//if ((otherNode->GetSourceNode().compare("Node3") == 0) and !otherNode->GetStatus())
				if ((otherNode->GetSourceNode().compare(Names::FindName(GetNode ()).c_str()) == 0) and !otherNode->GetStatus())
				{
					AddControllerNodeInfo(otherNode);
				}

				Ptr<Node> node1 = Names::Find<Node> (node->GetSourceNode());
				//Ptr<Node> node1 = GetNode();
				NS_ASSERT_MSG (node1 != 0, node->GetSourceNode() << "is not a Node");

				Ptr<L3Protocol> ndn1 = node1->GetObject<L3Protocol> ();
				NS_ASSERT_MSG (ndn1 != 0, "Ndn protocol hasn't been installed on a node, please install it first");

				shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace> (ndn1->getFaceById(atoi(fields[n+1].c_str())));
				node->AddIncidency(face, otherNode, atoi(fields[n+2].c_str()));
				//my_graph.add_incidency(node,otherNode,atof(fields[n+2].c_str()));
			}
	}
}


void ControllerApp::SchedulerHandlingFailureCalc(uint32_t seconds)
{
	cout <<"\n ControllerApp: SchedulerHandlingFailureCalc Called " << endl;
	std::string strUpdateString1 = "Source_Node_Name:Node2,Link_Update:{/Node3,257,5,1,FACE_DOWN}";
	std::string strUpdateString2 = "Source_Node_Name:Node3,Link_Update:{/Node2,257,5,1,FACE_DOWN}";

	cout <<"\n ControllerApp: SchedulerHandlingFailureCalc string 1 -> " << strUpdateString1 <<endl;
	cout <<"\n ControllerApp: SchedulerHandlingFailureCalc string 2 " << strUpdateString1 << endl;

	NdnControllerString strControllerUpdatedData = NdnControllerString(strUpdateString1);
	std::string strSourceNodeUpdate =	strControllerUpdatedData.GetSourceNode();
	Ptr<ControllerRouter> node = IsNodePresent(strSourceNodeUpdate);
	UpdateIncidency(node,strControllerUpdatedData.GetLinkUpdateInfo());

	NdnControllerString strControllerUpdatedData2 = NdnControllerString(strUpdateString2);
	std::string strSourceNodeUpdate1 =	strControllerUpdatedData2.GetSourceNode();
	Ptr<ControllerRouter> node1 = IsNodePresent(strSourceNodeUpdate1);
	UpdateIncidency(node1,strControllerUpdatedData2.GetLinkUpdateInfo());

	//CalculateRoutes();
	CalculateKPathYanAlgorithm(3); // Calling Yan's K path algorithm.
	StartSendingPathToNode(); // Start seding packets to individual nodes.
}


void ControllerApp::UpdateIncidency(Ptr<ControllerRouter> sourceNode, std::vector<string> fields)
{

	cout <<"\n ControllerApp: UpdateIncidency Called " << endl;
	if (!fields.empty() and fields.size() >= 4)
	{
		//Ptr<Node> node1 = GetNode();
		Ptr<Node> node1 = Names::Find<Node> (sourceNode->GetSourceNode());
		NS_ASSERT_MSG (node1 != 0,  sourceNode->GetSourceNode() << "is not a Node");

		Ptr<L3Protocol> ndn1 = node1->GetObject<L3Protocol> ();
		NS_ASSERT_MSG (ndn1 != 0, "Ndn protocol hasn't been installed on a node, please install it first");

		for (size_t n = 0; n < fields.size(); n+=5)
		{

			Ptr<ControllerRouter> otherNode = IsNodePresent(fields[n].substr(1,fields[n].length()));
			if(otherNode==NULL)
			{
				otherNode = CreateObject<ControllerRouter>(fields[n]);
				ns3::ndn::ControllerNodeList::Add(otherNode);
			}

			shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace>(ndn1->getFaceById(atoi(fields[n+1].c_str())));
			if (fields[n+4].compare("FACE_DOWN")==0)
			{
				// Remove link from the adjancies
				cout <<"\n Before updating the links for -> " << sourceNode->GetSourceNode() << endl;
				sourceNode->writelog();

				bool Isupdated = sourceNode->RemoveIncidency(face,otherNode,atoi(fields[n+2].c_str()));
				cout << "\n UpdateIncidency: RemoveIncidency status  " << Isupdated << endl;

				cout <<"\n After updating the links for -> " << sourceNode->GetSourceNode() << endl;
				sourceNode->writelog();
			}
			else if(fields[n+4].compare("FACE_UP")==0)
			{
				// Add link to the adjancies
				sourceNode->AddIncidency(face,otherNode,atoi(fields[n+2].c_str()));
			}
			else if(fields[n+4].compare("LINK_COST")==0)
			{
				// Update link cost only
				sourceNode->UpdateIncidency(face,otherNode,atoi(fields[n+2].c_str()));
			}
			else if(fields[n+4].compare("LINK_ADDED")==0)
			{
				// Add new link into adjancies
				sourceNode->AddIncidency(face,otherNode,atoi(fields[n+2].c_str()));
			}
			else if(fields[n+4].compare("LINK_REMOVED")==0)
			{
				// Remove link from adjancies
				sourceNode->RemoveIncidency(face,otherNode,atoi(fields[n+2].c_str()));
			}
			else
			{
				cout << "\n Couldn't interepreted the status of the link -> " << fields[3]<< endl;
			}
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

	/*
	uint32_t seq = std::numeric_limits<uint32_t>::max(); // invalid

	while (m_retxSeqs.size()) {
		seq = *m_retxSeqs.begin();
		m_retxSeqs.erase(m_retxSeqs.begin());
		break;
	}

	if (seq == std::numeric_limits<uint32_t>::max())
	{
		if (m_seqMax != std::numeric_limits<uint32_t>::max())
		{
			if (m_seq >= m_seqMax) {
				return; // we are totally done
			}
		}
	seq = m_seq++;
	}
*/
	shared_ptr<Name> nameWithSequence = make_shared<Name>(strPrefix);
	//nameWithSequence->appendSequenceNumber(seq);
	shared_ptr<Interest> interestConto = make_shared<Interest>();

	interestConto->setNonce(m_rand.GetValue());
	interestConto->setName(*nameWithSequence);

	time::milliseconds interestLifeTime(ndn::time::seconds(1000000));
	//interestConto->setInterestLifetime(interestLifeTime);

	//time::milliseconds interestLifeTime(m_interestLifeTime.GetMilliSeconds());
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

	std::map<Ptr<ControllerRouter>,std::list<std::tuple<std::shared_ptr<Name>,std::shared_ptr<Face>,size_t>>>::const_iterator iter;
	for (iter = dstNode->GetPathInfo().begin();iter!=dstNode->GetPathInfo().end();iter++)
	{
		strPath.append("(");
		strPath.append(iter->first->GetSourceNode());
		strPath.append(",");

		int count = 0;
		int lstSize = iter->second.size();
		std::list<std::tuple<std::shared_ptr<Name>,std::shared_ptr<Face>,size_t>> lstList = iter->second;

		for (auto& iter: lstList)
		{
			count++;
			strPath.append(std::get<0>(iter)->toUri().c_str());
			strPath.append(",");
			strPath.append(std::to_string(std::get<1>(iter)->getId()));
			strPath.append(",");
			strPath.append(std::to_string(std::get<2>(iter)));
			if(count != lstSize)
			{
				strPath.append(",");
			}
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
		//strPath = strForNode + "," + "/controller" + "," + "256" + "," + "3" + "," + "0";
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
	//dPacket->setFreshnessPeriod(ndn::time::milliseconds(6000));
	dPacket->setFreshnessPeriod(ndn::time::milliseconds(20000));
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

	std::cout << "\n ControllerApp: Data packet- > " << dPacket->getName () << " is sending from face -> " << m_face << std::endl;
	m_transmittedDatas(dPacket, this, m_face);
	m_face->onReceiveData(*dPacket);
	std::cout << "\n";
}


void ControllerApp::SendHelloDataPacket(shared_ptr<const Interest> interest) {

	  /* interest name: /<neighbor>/NLSR/INFO/<router> */
	  Name interestName = interest->getName();
	  //Name neighbor = interestName.getPrefix(-2);
	  Name neighbor = interestName.get(-3).toUri();
	  cout<<"Neighbor: " << neighbor << endl;

	  //m_adList.writeLog();
	  if (m_gb_adList.isNeighbor(neighbor))
	  {
		  std::cout << "\n ControllerApp: Sending Hello data packet- > " << interest->getName() << " is sending from face -> " << m_face->getId() << std::endl;
		  Name dataName(interest->getName());
		  auto dPacket = make_shared<Data>();
		  dPacket->setName(dataName);
		  //dPacket->setFreshnessPeriod(ndn::time::milliseconds(6000));
		  dPacket->setFreshnessPeriod(ndn::time::milliseconds(20000));
		  dPacket->setContent(reinterpret_cast<const uint8_t*>(INFO_COMPONENT.c_str()),
						INFO_COMPONENT.size());
		  Signature signature;
		  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));
		  if (m_keyLocator.size() > 0){
		  	signatureInfo.setKeyLocator(m_keyLocator);
		  }

		  signature.setInfo(signatureInfo);
		  signature.setValue(Block(&m_signature, sizeof(m_signature)));
		  dPacket->setSignature(signature);
		  dPacket->wireEncode();

		  m_transmittedDatas(dPacket, this, m_face);
		  m_face->onReceiveData(*dPacket);

		  Adjacent *adjacent = m_gb_adList.findAdjacent(neighbor);
		  if (adjacent->getStatus() == Adjacent::STATUS_INACTIVE)
		  {
			  if(adjacent->getFaceId() != 0)
			  {
				  Name interestName(neighbor) ;
				  interestName.append(Names::FindName(GetNode()));
				  interestName.append(HELLO_COMPONENT);
				  interestName.append(INFO_COMPONENT);
				  m_gb_adList.incrementInterestSendCount(neighbor);
				  expressInterest(interestName,m_conf.getInterestResendTime());
			  }
			  /*
			  else {
				  registerPrefixes(adjacent->getName(), adjacent->getConnectingFaceUri(),
							 	 adjacent->getLinkCost(), ndn::time::milliseconds::max());
			  }*/
		  }
	  }
}



void ControllerApp::OnInterest(std::shared_ptr<const Interest> interest) {
	App::OnInterest(interest); // tracing inside
	NS_LOG_FUNCTION(this << interest);
	std::cout << "\n CentralizedControllerApp: Received interest packet -> " << interest->getName() << std::endl;
	if (!m_active)
		return;

	std::string strPrefix;
	std::string strRequestType = extractNodeRequestType(interest->getName().toUri(),3);
	std::string strInterestNodePrefix = extractNodeName(interest->getName().toUri(),2);


	cout <<"\n Controller strRequestType -> " << strRequestType << endl;

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
		sendPathDataPacket(interest);

		if(strInterestNodePrefix.compare("Node3")==0)
		{
			//SchedulerHandlingFailureCalc();
			scheduleFailEvent(50);
		}
	}
	else if(strRequestType.compare("req_update") == 0)
	{
		strPrefix = "/" + strInterestNodePrefix + "/controller" + "/req_update";
		std::cout << "\n CentralizedControllerApp: Sending interest packet to  " << strInterestNodePrefix << "  with acknowledge interest -> " << strPrefix << std::endl;
		sendInterestPacket(strPrefix);
	}
	else if(strRequestType.compare(HELLO_COMPONENT) == 0)
	{
			//strPrefix = "/";
			//SendInterestPacket(strPrefix);
			SendHelloDataPacket(interest);
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
		if (source != NULL && source->GetSourceNode().compare("controller")!=0){
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

	std::string strRequestType = extractNodeRequestType(contentObject->getName().toUri(),3);

	if (strRequestType.compare("req_route") == 0)
	{

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

		//if(strSourceNode.compare("Node6") == 0)
		if(strSourceNode.compare("Node3") == 0)
		{
			//CalculateRoutes();
			//CalculateRoutesSinglePath();
			CalculateKPathYanAlgorithm(3); // Calling Yan's K path algorithm.
			StartSendingPathToNode(); // Start seding packets to individual nodes.
			//SchedulerHandlingFailureCalc();
		}
	}
	else if(strRequestType.compare("req_update") == 0)
	{
	   // We got the data packet for updating the routes.
	  NdnControllerString strControllerUpdatedData = NdnControllerString(msg);
	  std::string strSourceNodeUpdate =	strControllerUpdatedData.GetSourceNode();
	  std::cout << "\n Received data packet for updating information from node  ->  " << strSourceNodeUpdate << "\t  information is -> " << msg << endl;

	  Ptr<ControllerRouter> node = IsNodePresent(strSourceNodeUpdate);

	  if(node==NULL)
	  {
		  cout <<"\n Node wasn't been presented in the database, hence creating a new reference and adding node into controller's database" << endl;
		  node = CreateObject<ControllerRouter>(strSourceNodeUpdate);
		  size_t k = ns3::ndn::ControllerNodeList::Add(node);

		  AddIncidency(node, strControllerUpdatedData.GetLinkInfo());
		  AddPrefix(node, strControllerUpdatedData.GetNodePrefixInfo());
	  }
	  else
	  {
		  UpdateIncidency(node,strControllerUpdatedData.GetLinkUpdateInfo());
		  //CalculateRoutes();
		  if(strSourceNodeUpdate.compare("Node2") == 0)
		  {
			//CalculateRoutes();
			//CalculateKPathYanAlgorithm(3); // Calling Yan's K path algorithm.
			//StartSendingPathToNode(); // Start seding packets to individual nodes.
		  }
	  }

	}
	else if(strRequestType.compare(HELLO_COMPONENT) == 0)
	{
		// We got the data packet for updating the routes.
				std::string msg1(reinterpret_cast<const char*>(contentObject->getContent().value()),
									contentObject->getContent().value_size());
				std::cout << "\n Printing Data information -> " << msg1 << endl;
				//std::string strNeighbor = extractNodeRequestType(contentObject->getName().toUri(),1);
				//std::cout << "\n Printing strNeighbor information -> " << strNeighbor << endl;
				Name dataName = contentObject->getName();
				Name neighbor = dataName.getPrefix(-3);
				std::cout << "\n Printing Neighbor information -> " << neighbor << endl;
				Adjacent::Status oldStatus = m_gb_adList.getStatusOfNeighbor(neighbor);
				m_gb_adList.setStatusOfNeighbor(neighbor, Adjacent::STATUS_ACTIVE);
				m_gb_adList.incrementDataRcvCount(neighbor);
				Adjacent::Status newStatus = m_gb_adList.getStatusOfNeighbor(neighbor);
				if ((oldStatus - newStatus) != 0) {
					//Initiate Controller updating call
					// Build the database as well as controller synch
					cout << "\n Status has been changed for Neighbor " << neighbor << endl;
				}
				else
				{
					cout << "\n Status didn't change for Neighbor " << neighbor << endl;
					cout << "\n Data count for neighbor  " << neighbor << " is -> "<< m_gb_adList.getDataRcvCount(neighbor) << endl;
				}
				//std::cout << "\n Printing strNeighbor information -> " << neighbor << endl;
				m_gb_adList.writeLog();
	}
	else
	{
				// We got the data packet for updating the routes.
				std::string msg1(reinterpret_cast<const char*>(contentObject->getContent().value()),
									contentObject->getContent().value_size());
				std::cout << "\n pRINTING DATA FROM OTHE NODE -> " << msg << endl;
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
