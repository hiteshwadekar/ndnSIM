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
#include "ns3/ndnSIM/helper/ndn-link-control-helper.hpp"

namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE("ndn.CustConsumer");

using namespace std;
using namespace nfd;

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(CustConsumer);
const std::string CustConsumer::INFO_COMPONENT = "INFO";
const std::string CustConsumer::HELLO_COMPONENT = "HELLO";
int CustConsumer::counter = 0;

TypeId CustConsumer::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::ndn::CustConsumer").SetGroupName("Ndn").SetParent<App>().AddConstructor<
			CustConsumer>().AddAttribute("Prefix",
					"Prefix, cust consumer application prefix", StringValue("/"),
					MakeNameAccessor(&CustConsumer::m_prefix), MakeNameChecker())
					.AddAttribute("Frequency",
					"Frequency of interest packets", StringValue("1.0"),
					MakeDoubleAccessor(&CustConsumer::m_frequency),
					MakeDoubleChecker<double>())
					.AddAttribute("PayloadSize",
					"Virtual payload size for Content packets",
					UintegerValue(1024),
					MakeUintegerAccessor(&CustConsumer::m_virtualPayloadSize),
					MakeUintegerChecker<uint32_t>()).
					AddAttribute("Freshness",
					"Freshness of data packets, if 0, then unlimited freshness",
					TimeValue(Seconds(0)),
					MakeTimeAccessor(&CustConsumer::m_freshness),
					MakeTimeChecker())
					.AddAttribute("Signature",
					"Fake signature, 0 valid signature (default), other values application-specific",
					UintegerValue(0),
					MakeUintegerAccessor(&CustConsumer::m_signature),
					MakeUintegerChecker<uint32_t>())
					.AddAttribute("KeyLocator",
					"Name to be used for key locator.  If root, then key locator is not used",
					NameValue(), MakeNameAccessor(&CustConsumer::m_keyLocator),
					MakeNameChecker());

	return tid;
}

CustConsumer::CustConsumer()
: m_firstTimeHello(true)
{
	//NS_LOG_FUNCTION_NOARGS ();
	//m_seqMax = std::numeric_limits<uint32_t>::max();
}


// inherited from Application base class.
void CustConsumer::StartApplication() {
	NS_LOG_FUNCTION_NOARGS ();
	App::StartApplication();
	NS_LOG_DEBUG("NodeID: " << GetNode ()->GetId ());
	FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
	//std::cout<< "####################################### Collecting Link Information ###############################################################"<< std::endl;
	//std::cout << "\n";
	initialize();
	std::string strNodeName = Names::FindName(Ptr<Node>(GetNode ()));
	std::cout<< "####################################### Start Three Way Communication with ->  " << strNodeName << " Controller (Requesting routes)###############################################################"<< std::endl;
	std::cout << "\n";
	std::cout<< "CustConsumerApp: Sending an Interest Packets -> "<< "/controller/" + strNodeName + "/req_route" << std::endl;
	std::string strPrefixToController = "/controller/" + strNodeName + "/req_route";
	SendInterestPacket(strPrefixToController);
	 //Simulator::Schedule(Seconds(1.0), &CustConsumer::SendInterestPacket, this);

	// // make face green, so it will be used primarily
	// StaticCast<fib::FibImpl> (fib)->modify (fibEntry,
	//                                        ll::bind (&fib::Entry::UpdateStatus,
	//                                                ll::_1, m_face, fib::FaceMetric::NDN_FIB_GREEN));
}

void CustConsumer::StopApplication() {
	NS_LOG_FUNCTION_NOARGS ();
	//NS_ASSERT(GetNode()->GetObject<Fib>() != 0);
	App::StopApplication();
}

void CustConsumer::initialize()
{
	// Get all neighbor information
	m_gb_adList = CollectLinks();
	//scheduleHelloPacketEvent(30);
	//scheduleFailEvent(35);
	//schedulecheckLinkEvent(60);
}

void
CustConsumer::scheduleHelloPacketEvent(uint32_t seconds)
{
	m_helloEvent = scheduler::schedule(ndn::time::seconds(seconds),bind(&CustConsumer::sendScheduledHelloInterest, this, seconds));
}

void
CustConsumer::schedulecheckLinkEvent(uint32_t seconds)
{
	m_checkEvent = scheduler::schedule(ndn::time::seconds(seconds),bind(&CustConsumer::VerifyLinks, this, seconds));
}

void
CustConsumer::scheduleFailEvent(uint32_t seconds)
{
	cout <<"\n Called scheduleFailEvent"<<endl;
	m_failEvent = scheduler::schedule(ndn::time::seconds(seconds),bind(&ndn::LinkControlHelper::FailLinkByName, "Node2", "Node3"));
	//Simulator::Schedule(Seconds(10.0), ndn::LinkControlHelper::FailLink, node1, node2);
}

void
CustConsumer::scheduleBoostLinkCost(uint32_t seconds)
{
	m_boostLinkEvent = scheduler::schedule(ndn::time::seconds(seconds),bind(&CustConsumer::boostLinkCost, this, seconds));
}

void
CustConsumer::boostLinkCost(uint32_t sequenceNumber)
{
	std::shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace> (GetNode()->GetObject<ndn::L3Protocol>()->getFaceById(atoi("256")));
	face->setMetric(std::numeric_limits<uint32_t>::max());
	shared_ptr<Name> namePrefix = make_shared<Name>("/Node1");
	cout <<"\n Boost link cost called "<<endl;
	FibHelper::AddRoute(GetNode(),*namePrefix,face,std::numeric_limits<uint32_t>::max());
}


void
CustConsumer::OnTimeout(uint32_t sequenceNumber)
{
	cout << "\n OnTimeout Called " << endl;
}

void
CustConsumer::expressInterest(const Name& interestName, uint32_t seconds)
{
	std::cout<< "\n Expressing Hello Interest from consumer sending from face ->  " << m_face->getId() << "interest name ->  "<< interestName << std::endl;
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
}

void CustConsumer::sendScheduledHelloInterest(uint32_t seconds)
{

	counter++;
	cout <<"\n Called Consumer sendScheduledHelloInterest function ------- " <<endl;
	cout << "\n Source node for Hello packets->  " << Names::FindName(GetNode())<<endl;
	cout << "\n Printing list value before sending Hello packets " <<endl;
	//m_gb_adList = CollectLinks();
	//m_gb_adList.writeLog();
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
	      m_gb_adList.incrementRetryPacketCount((*it).getName());
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


std::string CustConsumer::extractNodeRequestType(std::string strPrefixName, int index) {
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


void CustConsumer::updateNodeLinkInfo(std::string strLinkInfo, bool isFirstTime) {
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

	if(isFirstTime)
	{
		scheduleHelloPacketEvent(10);
		schedulecheckLinkEvent(30);
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




void CustConsumer::VerifyLinks(uint32_t seconds)
{
	// For this algorithm, we will have to cover following cases
	// 1: Both list size, if yes compare each object with other
	// 2: if, not, then if current list size greater than the old one, that case new object should send to controller, and rest of the objects will compare each other
	// 3: if, current list is smaller than old one, then old's one object no more present as link, so need to send that object info to controller as link_removed
	cout <<"\n VeryfyLinks: Called for node -> " << Names::FindName(GetNode()).c_str()<<endl;
	cout <<"\n Printing global list values ->  " << endl;
	m_gb_adList.writeLog();


	std::string strNodeName = Names::FindName(Ptr<Node>(GetNode ()));
	if(strNodeName.compare("Node2")==0)
	{
		//scheduleBoostLinkCost(40);
		boostLinkCost(40);
	}


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
						if(!strUpdateToController.str().empty())
						{
							strUpdateToController << ",";
						}
						strUpdateToController << adj->getName()  << "," << adj->getFaceId() << "," << adj->getLinkCost() << "," << adj->getStatus() << "," << "FACE_DOWN";
					}
				}
				else
				{
					// send this (*it) information to controller.
					if(!strUpdateToController.str().empty())
					{
						strUpdateToController << ",";
					}
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
				if(!strUpdateToController.str().empty())
				{
					strUpdateToController << ",";
				}
				strUpdateToController << adj->getName()  << "," << adj->getFaceId() << "," << adj->getLinkCost() << "," << adj->getStatus() << "," << "LINK_ADDED";
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
								if(!strUpdateToController.str().empty())
								{
									strUpdateToController << ",";
								}
								strUpdateToController << it1->getName()  << "," << it1->getFaceId() << "," << it1->getLinkCost() << "," << it1->getStatus() << "," << "FACE_DOWN";
							}
						}
						else
						{
							if(!strUpdateToController.str().empty())
							{
								strUpdateToController << ",";
							}
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
		ControllerSync(strUpdateToController);
	}
	m_gb_adList.reset();
	m_gb_adList.setAdjList(adjList1);
}

void CustConsumer::ControllerSync(std::stringstream& strUpdateToController)
{
	// Check first in FIB if current prefix has available in case if prefix has down or cost increased.
	// Prepare string for controller send.

	cout <<"\n ControllerSync: Called " << endl;
	cout <<"\n Printing values for controller -> " << strUpdateToController.str() << "   and source node name -> "<< Names::FindName(GetNode()).c_str() <<endl;
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
				Name NeighborName(fields[n]);
				std::cout << "\n ControllerSync: Printing getRetrycout ->   "<< m_gb_adList.getRetryPacketCount(NeighborName) << std::endl;
				/*if(m_gb_adList.getRetryPacketCount(NeighborName) > 8)
				{
					if(!strmodifiedControllerData.str().empty())
					{
						strmodifiedControllerData << ",";
					}
					strmodifiedControllerData << fields[n]  << "," << fields[n+1] << "," << fields[n+2] << "," << fields[n+3] << "," << fields[n+4];
					m_gb_adList.insertRetryPacketCount(NeighborName,0);
				}
				else
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
					}*/
				    //if(NextHopcounter==0)
				    //{
						if(!strmodifiedControllerData.str().empty())
						{
							strmodifiedControllerData << ",";
						}
						strmodifiedControllerData << fields[n]  << "," << fields[n+1] << "," << fields[n+2] << "," << fields[n+3] << "," << fields[n+4];
						//if(n <= fields.size())
						//{
							//strmodifiedControllerData << ",";
						//}
					//}
				//}
			}
		}
	}

	if(!strmodifiedControllerData.str().empty())
	{
		cout << "\n Final link info to controller 1 -> " << strmodifiedControllerData.str() << endl;
		if(!m_strUpdateToController.str().empty())
		{
			m_strUpdateToController <<",";
		}
		m_strUpdateToController << strmodifiedControllerData.str();
		m_strUpdateToController1 = strmodifiedControllerData.str();

		cout << "\n Final link info to controller 2  -> " << m_strUpdateToController.str() << endl;
		cout << "\n Final link info to controller 2  -> " << m_strUpdateToController1 << endl;
		unregisterPrefix(m_strUpdateToController1);
		std::string strControllerUpdate = "/controller/" + Names::FindName(Ptr<Node>(GetNode())) + "/req_update";
		SendInterestPacket(strControllerUpdate);
	}
	else
	{
		scheduleHelloPacketEvent(10);
		schedulecheckLinkEvent(20);
	}

}

void
CustConsumer::unregisterPrefix(std::string strLinkInfo)
{
	std::cout <<"\n UnregisterPrefix called " <<std::endl;
/*
  if (faceId > 0) {
    ControlParameters controlParameters;
    controlParameters
      .setName(namePrefix)
      .setFaceId(faceId)
      .setOrigin(128);
    FibHelper::RemoveNextHop(controlParameters,GetNode());
  }
*/
  //  /Node2,258,2,1,FACE_DOWN



  Ptr<Node> localNode = GetNode();
  Ptr<ndn::L3Protocol> l3 = localNode->GetObject<ndn::L3Protocol>();
  std::shared_ptr<ndn::nfd::Forwarder> fw = l3->getForwarder();
  ndn::nfd::Fib& fib = fw->getFib();
  std::vector<std::string> fields;
  boost::algorithm::split(fields, strLinkInfo, boost::algorithm::is_any_of(","));
  for (size_t n = 0; n < fields.size(); n+=5)
  {

	  /*
	  if (l3->getFaceById(atoi(fields[n+1].c_str()))->getId() > 0) {
		  ControlParameters controlParameters;
		  Name namePrefix(fields[n]);
		  controlParameters
	        .setName(namePrefix)
	        .setFaceId(l3->getFaceById(atoi(fields[n+1].c_str()))->getId())
	      	.setOrigin(128);
		  std::cout <<"\n FIB removed called " <<std::endl;
		  FibHelper::RemoveNextHop(controlParameters,GetNode());
	    }*/

	  Name namePrefix(fields[n]);
	  Name namePrefix_producer("/Producer");

	  //shared_ptr<fib::Entry> fibEntry = fib.findExactMatch(namePrefix);
	  //shared_ptr<fib::Entry> fibEntry1 = fib.findExactMatch(namePrefix_producer);
	  std::cout <<"\n FIB removed called for prefix ->  " <<  fields[n] <<  " and face id is ->  " <<  fields[n+1] << std::endl;
	  //fibEntry->removeNextHop(l3->getFaceById(atoi(fields[n+1].c_str())));
	  //fibEntry1->removeNextHop(l3->getFaceById(atoi(fields[n+1].c_str())));

	  std::shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace> (GetNode()->GetObject<ndn::L3Protocol>()->getFaceById(atoi(fields[n+1].c_str())));
	  IsFIBMetricsUpdatable(fields[n].c_str(),face,10);
	  //fib.removeNextHopFromAllEntries(l3->getFaceById(atoi(fields[n+1].c_str())));
	  fib.removeNextHopFromAllEntries(face);
	  ndn::FibHelper::AddRoute("Node2", "/controller", "Node3", 0);
	  IsFIBMetricsUpdatable(fields[n].c_str(),face,10);
	  /*
	  for (auto& fibEntry : fib) {
	  		std::string strTempString = fibEntry.getPrefix().toUri().c_str();
	  		if(strTempString.compare(fields[n]) == 0)
			{
				for (const auto& nh : fibEntry.getNextHops())
				{
					//shared_ptr<Face> face = l3->getFaceById(atoi(fields[n+1].c_str()));
					if(nh.getFace()->getId() == l3->getFaceById(atoi(fields[n+1].c_str()))->getId())
					{
						//nh.removeNextHop(l3->getFaceById(atoi(fields[n+1].c_str())));
						fibEntry.removeNextHop(l3->getFaceById(atoi(fields[n+1].c_str())));
					}
					//std::cout << "  - " << nh.getFace() << ", " << nh.getFace()->getId() << ", " << nh.getCost() << std::endl;
				}
			}
	  }*/
  }
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
	    			  std::cout <<"\n Face "<<  face->getId() << " is acitve and cost is " << face->getMetric() << std::endl;
	    		  }
	    		  else if(!face->isUp())
	    		  {
	    			  objAdjacent.setStatus(Adjacent::STATUS_INACTIVE);
	    			  std::cout <<"\n Face "<<  face->getId() << " is inacitve and cost is " << face->getMetric() <<std::endl;
	    		  }
	    		  else
	    		  {
	    			  objAdjacent.setStatus(Adjacent::STATUS_UNKNOWN);
	    			  std::cout <<"\n Face "<<  face->getId() << " is unknown and and cost is " << face->getMetric() << std::endl;
	    		  }
	    		  objAdjacent.setInterestSentNo(0);
	    		  objAdjacent.setDataRcvNo(0);
	    		  objAList.insert(objAdjacent);
	    		  objAList.insertRetryPacketCount(objAdjacent.getName(),0);
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



void CustConsumer::SendUpdateDataPacketToController(shared_ptr<const Interest> interest) {

	if (!m_active)
		return;
	NS_LOG_FUNCTION_NOARGS ();
	NS_LOG_FUNCTION(this << interest);

	std::cout<< "CustConsumerApp: Sending local changed into data Packet to controller-> "<< interest->getName() << std::endl;
	Name dataName(interest->getName());
	auto dPacket = make_shared<Data>();
	dPacket->setName(dataName);
	dPacket->setFreshnessPeriod(ndn::time::milliseconds(10000));


	NdnControllerString strControllerData = NdnControllerString("");
	strControllerData.SetSourceNode(Names::FindName(GetNode()).c_str());
	//strControllerData.SetLinkUpdateInfo(m_strUpdateToController.str());
	strControllerData.SetLinkUpdateInfo(m_strUpdateToController1);
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

void CustConsumer::SendDataPacket(shared_ptr<const Interest> interest, bool toController) {
	if (!m_active)
		return;
	NS_LOG_FUNCTION_NOARGS ();
	NS_LOG_FUNCTION(this << interest);

	//std::cout<< "CustConsumerApp: Sending a Data Packet -> "<< interest->getName() << std::endl;
	Name dataName(interest->getName());
	auto dPacket = make_shared<Data>();
	dPacket->setName(dataName);
	dPacket->setFreshnessPeriod(ndn::time::milliseconds(10000));

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


void CustConsumer::SendHelloDataPacket(shared_ptr<const Interest> interest) {

	  /* interest name: /<neighbor>/NLSR/INFO/<router> */
	  Name interestName = interest->getName();
	  //Name neighbor = interestName.getPrefix(-2);
	  Name neighbor = interestName.get(-3).toUri();
	  cout<<"Neighbor: " << neighbor << endl;

	  //m_adList.writeLog();
	  if (m_gb_adList.isNeighbor(neighbor))
	  {
		  std::cout << "\n CustConsumerApp: Sending Hello data packet- > " << interest->getName() << " is sending from face -> " << m_face->getId() << std::endl;
		  Name dataName(interest->getName());
		  auto dPacket = make_shared<Data>();
		  dPacket->setName(dataName);
		  dPacket->setFreshnessPeriod(ndn::time::milliseconds(10000));
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

void CustConsumer::sendAckDataPacket(std::shared_ptr<const Interest> interest)
{
	if (!m_active)
		return;
	NS_LOG_FUNCTION_NOARGS ();
	NS_LOG_FUNCTION(this << interest);

	Name dataName(interest->getName());
	auto dPacket = make_shared<Data>();
	dPacket->setName(dataName);
	//dPacket->setFreshnessPeriod(ndn::time::milliseconds(6000));
	dPacket->setFreshnessPeriod(ndn::time::milliseconds(20000));
	//std::string strTemplateNode = getTheCalculationPath(extractNodeName(interest->getName().toUri(), 2));
	//dPacket->setContent(reinterpret_cast<const uint8_t*>(strTemplateNode.c_str()), (uint32_t) strTemplateNode.length());
	Signature signature;
	SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));
	if (m_keyLocator.size() > 0) {
		signatureInfo.setKeyLocator(m_keyLocator);
	}
	signature.setInfo(signatureInfo);
	signature.setValue(Block(&m_signature, sizeof(m_signature)));

	dPacket->setSignature(signature);
	dPacket->wireEncode();

	std::cout << "\n CustConsumer: Sending ACK Data packet- > " << dPacket->getName () << " is sending from face -> " << m_face << std::endl;
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
	std::string strRequestType = extractNodeRequestType(interest->getName().toUri(),3);

	cout <<"\n Request interest type is -> " << strRequestType << endl;

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
		sendAckDataPacket(interest);
		SendInterestPacket(strPrefix);
	}
	else if(strRequestType.compare(HELLO_COMPONENT) == 0)
	{
		//strPrefix = "/";
		//SendInterestPacket(strPrefix);
		SendHelloDataPacket(interest);
	}
	else if(strRequestType.compare("req_update") == 0)
	{
		SendUpdateDataPacketToController(interest);
	}
	else if(strRequestType.compare("res_updated_path")==0)
	{
		strPrefix = "/controller/" + strNodeName + "/res_updated_path";
		std::cout << "\n CustConsumerApp: Sending interest packet to controller -> " << strPrefix << std::endl;
		sendAckDataPacket(interest);
		SendInterestPacket(strPrefix);
	}
	else if(strRequestType.compare("producer_test")==0)
	{
		std::cout << "\n CustConsumerApp: Sending Data packet to Consumer -> " << interest->getName().toUri() << std::endl;
		SendDataPacket(interest, false);
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

	std::string strRequestType = extractNodeRequestType(contentObject->getName().toUri(),3);
	std::string strNodeName = Names::FindName(Ptr<Node>(GetNode ()));
	std::string strPrefix = "";
	if (strRequestType.compare("req_route") == 0)
	{
		//Do nothing...really? oowoow.
		std::cout <<"\n Received ACK from controller req_route"<<std::endl;
	}
	else if(strRequestType.compare("res_route") == 0)
	{
		// We got the data packet for updating the routes.
		std::string msg(reinterpret_cast<const char*>(contentObject->getContent().value()),
					contentObject->getContent().value_size());
		updateNodeLinkInfo(msg,true);
		std::cout << "\n" << endl;
		std::cout<< "####################################### Stop Three Way Communication with Controller ###############################################################"<< std::endl;
		/*
		std::string strNode="";
		std::cout<< "####################################### Testing updated paths by sending interest packets to each other ###############################################################"<< std::endl;
		for (ns3::NodeList::Iterator node = ns3::NodeList::Begin(); node != ns3::NodeList::End();
		         node++) {
			strNode = Names::FindName(Ptr<Node>(*node));
			if(strNode.compare("controller")!=0 && strNode.compare(strNodeName)!=0)
			{
				strNode = "/" + strNode;
				SendInterestPacket(strNode);
			}
		}*/
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
			VerifyLinks(30);
		}
		else
		{
			cout << "\n Status didn't change for Neighbor " << neighbor << endl;
			cout << "\n Data count for neighbor  " << neighbor << " is -> "<< m_gb_adList.getDataRcvCount(neighbor) << endl;
		}

		//m_gb_adList.writeLog();

		//std::cout << "\n Printing strNeighbor information -> " << neighbor << endl;
	}
	else if(strRequestType.compare("req_update") == 0)
	{
		//Do nothing...really? oowoow.
		std::cout <<"\n Received ACK from controller for req_update"<<std::endl;
	}
	else if(strRequestType.compare("res_updated_path") == 0)
	{
		// We got the updated data packet for updating the routes.
		std::string msg(reinterpret_cast<const char*>(contentObject->getContent().value()),
					contentObject->getContent().value_size());
		updateNodeLinkInfo(msg,true);
		std::cout << "\n" << endl;
		/*
		std::string strNode="";
		std::cout<< "####################################### Testing updated paths by sending interest packets to each other ###############################################################"<< std::endl;
		for (ns3::NodeList::Iterator node = ns3::NodeList::Begin(); node != ns3::NodeList::End();
				 node++) {
			strNode = Names::FindName(Ptr<Node>(*node));
			if(strNode.compare("controller")!=0 && strNode.compare(strNodeName)!=0)
			{
				strNode = "/" + strNode;
				SendInterestPacket(strNode);
			}
		}*/
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
