#include "ndn-custconsumer.hpp"
#include "ns3/log.h"
//#include "ns3/ndn-interest.h"
//#include "ns3/ndn-data.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/names.h"

#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/double.h"

//#include "ns3/ndn-app-face.h"
//#include "ns3/ndn-fib.h"

#include "string.h"
#include <map>
#include <iostream>

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"
#include "model/ndn-net-device-face.hpp"

#include "helper/ndn-controller-string-parser.hpp"

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


#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"


#include <boost/ref.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/algorithm/string.hpp>

#include "helper/ndn-app-prefix-helper.hpp"

namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE("ndn.CustConsumer");

using namespace std;
using namespace nfd;

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(CustConsumer);
const std::string CustConsumer::INFO_COMPONENT = "INFO";
const std::string CustConsumer::HELLO_COMPONENT = "HELLO";

TypeId CustConsumer::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::ndn::CustConsumer").SetGroupName("Ndn").SetParent<App>().AddConstructor<
			CustConsumer>().AddAttribute("Prefix",
					"Prefix, cust consumer application prefix", StringValue("/"),
					MakeNameAccessor(&CustConsumer::m_prefix), MakeNameChecker())
					.AddAttribute("Frequency",
					"Frequency of interest packets", StringValue("1.0"),
					MakeDoubleAccessor(&CustConsumer::m_frequency),
					MakeDoubleChecker<double>()).AddAttribute("PayloadSize",
					"Virtual payload size for Content packets",
					UintegerValue(1024),
					MakeUintegerAccessor(&CustConsumer::m_virtualPayloadSize),
					MakeUintegerChecker<uint32_t>()).AddAttribute("Freshness",
					"Freshness of data packets, if 0, then unlimited freshness",
					TimeValue(Seconds(0)),
					MakeTimeAccessor(&CustConsumer::m_freshness),
					MakeTimeChecker()).AddAttribute("Signature",
					"Fake signature, 0 valid signature (default), other values application-specific",
					UintegerValue(0),
					MakeUintegerAccessor(&CustConsumer::m_signature),
					MakeUintegerChecker<uint32_t>()).AddAttribute("KeyLocator",
					"Name to be used for key locator.  If root, then key locator is not used",
					NameValue(), MakeNameAccessor(&CustConsumer::m_keyLocator),
					MakeNameChecker());

	return tid;
}

CustConsumer::CustConsumer(){
	//NS_LOG_FUNCTION_NOARGS ();
	//m_seqMax = std::numeric_limits<uint32_t>::max();
}


// inherited from Application base class.
void CustConsumer::StartApplication() {
	NS_LOG_FUNCTION_NOARGS ();
	App::StartApplication();
	NS_LOG_DEBUG("NodeID: " << GetNode ()->GetId ());

	//Ptr<Fib> fib = GetNode()->GetObject<Fib>();
	//Ptr<fib::Entry> fibEntry = fib->Add(m_prefix, m_face, 0);

	//fibEntry->UpdateStatus(m_face, fib::FaceMetric::NDN_FIB_GREEN);
	FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);

	std::cout<< "####################################### Collecting Link Information ###############################################################"<< std::endl;
	initialize();
	std::cout << "\n";
	std::cout<< "####################################### Start Three Way Communication with Controller (Requesting routes)###############################################################"<< std::endl;
	std::cout << "\n";
	std::string strNodeName = Names::FindName(Ptr<Node>(GetNode ()));
	std::cout<< "CustConsumerApp: Sending an Interest Packets -> "<< "/controller/" + strNodeName + "/req_route" << std::endl;
	std::string strPrefixToController = "/controller/" + strNodeName + "/req_route";
	SendInterestPacket(strPrefixToController);
	 //Simulator::Schedule(Seconds(1.0), &CustConsumer::SendInterestPacket, this);

	// // make face green, so it will be used primarily
	// StaticCast<fib::FibImpl> (fib)->modify (fibEntry,
	//                                        ll::bind (&fib::Entry::UpdateStatus,
	//                                                  ll::_1, m_face, fib::FaceMetric::NDN_FIB_GREEN));
}

void CustConsumer::StopApplication() {
	NS_LOG_FUNCTION_NOARGS ();
	//NS_ASSERT(GetNode()->GetObject<Fib>() != 0);

	App::StopApplication();
}

void CustConsumer::initialize()
{
	// Get all neighbor information
	m_adList = CollectLinks();
	scheduleHelloPacketEvent(100);
}

void CustConsumer::scheduleHelloPacketEvent(uint32_t seconds)
{
	cout <<"\n Called scheduleHelloPacketEvent function ------- " <<endl;
	scheduler::schedule(ndn::time::seconds(seconds),bind(&CustConsumer::sendScheduledHelloInterest, this, seconds));
}


void
CustConsumer::OnTimeout(uint32_t sequenceNumber)
{



}


void
CustConsumer::expressInterest(const Name& interestName, uint32_t seconds)
{
	cout<< "\n Expressing Hello Interest :" << interestName << endl;
  	//Interest i(interestName);
  	//i.setInterestLifetime(ndn::time::seconds(seconds));
  	//i.setMustBeFresh(true);

  	shared_ptr<Interest> interestConto = make_shared<Interest>();
  	//nameWithSequence->append(m_postfix);
  	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
  	interestConto->setNonce(m_rand.GetValue());
  	interestConto->setName(interestName);
  	time::milliseconds interestLifeTime(ndn::time::seconds(1000000));
  	interestConto->setInterestLifetime(interestLifeTime);
  	m_transmittedInterests(interestConto, this, m_face);
  	m_face->onReceiveInterest(*interestConto);
  	std::cout << "\n";
  //m_face.expressInterest(i,ndn::bind(&HelloProtocol::onContent,this,_1, _2),ndn::bind(&HelloProtocol::processInterestTimedOut,
   //                                           this, _1));

}

void CustConsumer::sendScheduledHelloInterest(uint32_t seconds)
{
	cout <<"\n Called sendScheduledHelloInterest function ------- " <<endl;

	AdjacencyList adList = CollectLinks();
	std::list<Adjacent> adjList = CollectLinks().getAdjList();

	for (std::list<Adjacent>::iterator it = adjList.begin(); it != adjList.end();
	       ++it) {
	    if((*it).getFaceId() != 0) {
	      /* interest name: /<neighbor>/HELLO/INFO/ */
	      Name interestName = (*it).getName() ;
	      interestName.append(Names::FindName(GetNode()));
	      interestName.append(HELLO_COMPONENT);
	      interestName.append(INFO_COMPONENT);
	      expressInterest(interestName,m_conf.getInterestResendTime());
	    }
	    /*
	    else {
	      registerPrefixes((*it).getName(), (*it).getConnectingFaceUri(),
	                       (*it).getLinkCost(), ndn::time::milliseconds::max());
	    }*/
	  }
	scheduleHelloPacketEvent(m_conf.getInfoInterestInterval());
}


std::string CustConsumer::extractNodeRequestType(std::string strPrefixName) {
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strPrefixName,
			boost::algorithm::is_any_of("/"));

	//for (size_t n = 0; n < fields.size(); n++)
	//	std::cout << fields[n] << "\"\n";
	//cout << endl;

	if (fields.size()>=3)
	{
		return fields[3];
	}
	else
	{
		return fields[0];
	}
}


bool CustConsumer::IsFIBMetricsUpdatable(std::string strPrefixName,std::shared_ptr<NetDeviceFace> faceId,size_t faceMetrics)
{
	bool IsRequireUpdate=false;
	Ptr<ndn::L3Protocol> l3 = GetNode()->GetObject<ndn::L3Protocol>();
	std::shared_ptr<ndn::nfd::Forwarder> fw = l3->getForwarder();
	ndn::nfd::Fib& fib = fw->getFib();
	for (const auto& fibEntry : fib) {
		std::string strTempString = fibEntry.getPrefix().toUri().c_str();
		//std::cout << "  CustConsumer Prefix Name -> " << fibEntry.getPrefix() << std::endl;
		//if(strTempString.compare(strPrefixName)== 0)
		//{
			for (const auto& nh : fibEntry.getNextHops())
			{
				std::cout << "  CustConsumer Prefix Name -> " << strTempString << " - " << nh.getFace() << ", " << nh.getFace()->getId() << ", " << nh.getCost() << "," << nh.getFace()->getMetric() << std::endl;
			}
		//}
	  //std::cout << "  -" << fibEntry.getPrefix() << std::endl;
	  //std::cout << "Next hop: " << std::endl;
	  //for (const auto& nh : fibEntry.getNextHops()) {
		//std::cout << "    - " << nh.getFace() << ", " << nh.getFace()->getId() << ", " << nh.getCost() << std::endl;
	  //}
	}
	return IsRequireUpdate;
}




void CustConsumer::updateNodeLinkInfo(std::string strLinkInfo) {
	// Update the FIB and face metrics with calculated distance by controller.
	std::cout<<"\n";
	std::cout << "CustConsumer:: (updateNodeLinkInfo): Updating FIB with the provided information "<<std::endl;
	cout << "Packet Data ->  "<< strLinkInfo <<endl;

	NdnControllerString strControllerData = NdnControllerString(strLinkInfo);

	std::vector<std::string> fields;
	fields = strControllerData.GetCalculatedPathInfo();

	for (size_t n = 0; n < fields.size(); n+=1)
	{
		if(!fields[n].empty())
		{
			std::cout << "\n Data Path -> " << fields[n] <<std::endl;
			std::vector<std::string> prefixMetrics;
				boost::algorithm::split(prefixMetrics, fields[n],
						boost::algorithm::is_any_of(","));
			for (size_t k = 0; k < prefixMetrics.size()-1; k+=3)
			{
				std::cout << "\t\t\t \n Splitted Data -> [" << k << "] " << prefixMetrics[k] <<std::endl;
				std::cout << "\t\t\t \n Splitted Data -> [" << k+1 << "] " << prefixMetrics[k+1] <<std::endl;
				std::cout << "\t\t\t \n Splitted Data -> [" << k+2 << "] " << prefixMetrics[k+2] <<std::endl;
				std::cout << "\t\t\t \n Splitted Data -> [" << k+3 << "] " << prefixMetrics[k+3] <<std::endl;
				std::shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace> (GetNode()->GetObject<ndn::L3Protocol>()->getFaceById(atoi(prefixMetrics[k+2].c_str())));
				std::cout << "\t\t\t \n FaeMetrics ->  " << face->getMetric() <<std::endl;

				std::cout << "\t\t\t \n Before Updating the FIB -> " << std::endl;
				IsFIBMetricsUpdatable(prefixMetrics[k+1],face,atoi(prefixMetrics[k+3].c_str()));

				shared_ptr<Name> namePrefix = make_shared<Name>(prefixMetrics[k+1]);
				//FibHelper::AddRoute(GetNode(),*namePrefix,face,face->getMetric());
				FibHelper::AddRoute(GetNode(),*namePrefix,face,atoi(prefixMetrics[k+3].c_str()));

				std::cout << "\t\t\t \n After Updating the FIB -> " << std::endl;
				IsFIBMetricsUpdatable(prefixMetrics[k+1],face,atoi(prefixMetrics[k+3].c_str()));

			}
		}
	}

	std::cout << "\n ******* ****************************** Stopping Controller to Consumer Communication ************************************************************"<<std::endl;

	// call FIB control command from NFD to update the fib check the status.

}


void CustConsumer::SendInterestPacket(std::string strPrefixToController) {
	if (!m_active)
		return;
	NS_LOG_FUNCTION_NOARGS ();
	shared_ptr<Name> nameWithSequence = make_shared<Name>(strPrefixToController);
	//Ptr<Interest> interestConto = Create<Interest>();
	shared_ptr<Interest> interestConto = make_shared<Interest>();
	//nameWithSequence->append(m_postfix);
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	interestConto->setNonce(m_rand.GetValue());
	interestConto->setName(*nameWithSequence);
	time::milliseconds interestLifeTime(ndn::time::seconds(1000000));
	interestConto->setInterestLifetime(interestLifeTime);
	m_transmittedInterests(interestConto, this, m_face);
	m_face->onReceiveInterest(*interestConto);
	std::cout << "\n";
}


std::string CustConsumer::getPrefix(Ptr<Node> NodeObj)
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
	TypeId m_tid = this->GetTypeId();
	Ptr<Application> app;
	cout << "Node Name -> "<< Names::FindName(NodeObj)<<endl;
	cout << "Number of application -> "<< NodeObj->GetNApplications()<<endl;
	for (uint32_t i = 0; i < NodeObj->GetNApplications(); i++)
	{
	  app = NodeObj->GetApplication(i);

	  cout << "\n App Id -> "<< app->GetTypeId();
	  cout << "\n m_tid -> "<<m_tid;
	  TypeId m_apptid = app->GetTypeId();
	 // if (m_apptid == m_tid)
	  //{
		 ObjectFactory factory;
		 factory.SetTypeId (m_tid);
		 cout << "Factory -> " << factory << endl;
	  //}
	}
	*/

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
	  //std::cout << "  -" << fibEntry.getPrefix() << std::endl;
	  //std::cout << "Next hop: " << std::endl;
	  //for (const auto& nh : fibEntry.getNextHops()) {
	    //std::cout << "    - " << nh.getFace() << ", " << nh.getFace()->getId() << ", " << nh.getCost() << std::endl;
	  //}
	}
	*/
	return attrValue;
}


AdjacencyList CustConsumer::CollectLinks()
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
	    		  objAList.insert(objAdjacent);
		    }
		}
	 }
	return objAList;
}


std::string CustConsumer::GetLocalLinkInfo()
{
	std::stringstream strStateTemplate;
	//std::string strStateTemplate;
	bool firstVisit = false;

	Ptr<Node> localNode = GetNode ();
	//cout << "\n CustConsumerApp: Collecting Local Link Information of Node -> " << Names::FindName(localNode);
	Ptr<L3Protocol> ndn = localNode->GetObject<L3Protocol> ();
	NS_ASSERT_MSG (ndn != 0, "Ndn protocol hasn't been installed on a node, please install it first");

	NdnControllerString strControllerData = NdnControllerString("");
	strControllerData.SetSourceNode(Names::FindName(localNode).c_str());

	for (auto& faceId : ndn->getForwarder()->getFaceTable())
	 //for (uint32_t faceId = 0; faceId < ndn->GetNFaces (); faceId++)
	    {
	      //Ptr<NetDeviceFace> face = DynamicCast<NetDeviceFace> (ndn->GetFace (faceId));
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
		      if (nd == otherSide) continue;
		      Ptr<Node> otherNode = otherSide->GetNode ();
		      NS_ASSERT (otherNode != 0);
		      Ptr<L3Protocol> otherNdn = otherNode->GetObject<L3Protocol> ();
		      NS_ASSERT_MSG (otherNdn != 0, "Ndn protocol hasn't been installed on the other node, please install it first");

		      cout <<"\n Face Status " << face->getFaceStatus() <<endl;

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


void CustConsumer::SendDataPacket(shared_ptr<const Interest> interest, bool toController) {
	if (!m_active)
		return;
	NS_LOG_FUNCTION_NOARGS ();
	NS_LOG_FUNCTION(this << interest);

	std::cout<< "CustConsumerApp: Sending a Data Packet -> "<< interest->getName() << std::endl;
	Name dataName(interest->getName());
	auto dPacket = make_shared<Data>();
	dPacket->setName(dataName);
	dPacket->setFreshnessPeriod(ndn::time::milliseconds(6000));

	std::string strTemplateNode="";
	if(toController)
	{
		strTemplateNode = GetLocalLinkInfo();
	}
	else
	{
		strTemplateNode = "Hello from -> ";
		strTemplateNode.append(Names::FindName(Ptr<Node>(GetNode ())));
	}
	dPacket->setContent(reinterpret_cast<const uint8_t*>(strTemplateNode.c_str()), (uint32_t) strTemplateNode.length());
	//ndn::StackHelper::getKeyChain().sign(*dPacket);
	 Signature signature;
	 SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));
	 if (m_keyLocator.size() > 0) {
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

string CustConsumer::extractNodeName(std::string strPacketName) {
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strPacketName,
			boost::algorithm::is_any_of("/"));
	return fields[2];
}

void CustConsumer::OnInterest(shared_ptr<const Interest> interest) {
	App::OnInterest(interest); // tracing inside
	if (!m_active)
		return;
	NS_LOG_FUNCTION(this << interest);
	std::cout << "\n CustConsumerApp: Received Interest Packet -> " << interest->getName() << std::endl;
	std::string strRequestType = extractNodeRequestType(interest->getName().toUri());
	std::string strNodeName = Names::FindName(Ptr<Node>(GetNode ()));
	std::string strPrefix = "";

	if (strRequestType.compare("req_route") == 0)
	{
		SendDataPacket(interest, true);
	}
	else if(strRequestType.compare("res_route") == 0)
	{
		strPrefix = "/controller/" + strNodeName + "/res_route";
		std::cout << "\n CustConsumerApp: Sending interest packet ack to controller -> " << strPrefix << std::endl;
		SendInterestPacket(strPrefix);
	}
	else
	{
		//strPrefix = "/";
		//SendInterestPacket(strPrefix);
		SendDataPacket(interest, false);
	}

}

void CustConsumer::OnData(shared_ptr<const Data> contentObject) {
	App::OnData(contentObject); // tracing inside
	NS_LOG_FUNCTION(this << contentObject);
	std::cout << "\n CustConsumerApp: Received Data packet -> "
			<< contentObject->getName() << std::endl;

	std::string strRequestType = extractNodeRequestType(contentObject->getName().toUri());
	std::string strNodeName = Names::FindName(Ptr<Node>(GetNode ()));
	std::string strPrefix = "";
	if (strRequestType.compare("req_route") == 0)
	{
		//Do nothing...really? oowoow.
	}
	else if(strRequestType.compare("res_route") == 0)
	{
		// We got the data packet for updating the routes.
		std::string msg(reinterpret_cast<const char*>(contentObject->getContent().value()),
					contentObject->getContent().value_size());
		updateNodeLinkInfo(msg);
		std::cout << "\n" << endl;
		std::cout<< "####################################### Stop Three Way Communication with Controller ###############################################################"<< std::endl;


		std::string strNode="";
		std::cout<< "####################################### Testing updated paths by sending interest packets to each other ###############################################################"<< std::endl;
		for (ns3::NodeList::Iterator node = ns3::NodeList::Begin(); node != ns3::NodeList::End();
		         node++) {
			strNode = Names::FindName(Ptr<Node>(*node));
			if(strNode.compare("Node1")!=0 && strNode.compare(strNodeName)!=0)
			{
				strNode = "/" + strNode;
				SendInterestPacket(strNode);
			}
		}
	}
	else
	{
		// We got the data packet for updating the routes.
		std::string msg1(reinterpret_cast<const char*>(contentObject->getContent().value()),
							contentObject->getContent().value_size());
		std::cout << "\n pRINTING DATA FROM OTHE NODE -> " << msg1 << endl;
	}
	std::cout << "\n";
}

void CustConsumer::OnNack(shared_ptr<const Interest> interest) {
	//App::OnNack(interest); // tracing inside
	NS_LOG_FUNCTION(this << interest);
	std::cout << "\n CustConsumerApp: Received Nack packet -> "
			<< interest->getName() << std::endl;
}

} // namespace ndn
}

// namespace ns3
