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
	scheduleHelloPacketEvent(100);
}

void
ControllerApp::scheduleHelloPacketEvent(uint32_t seconds)
{
	cout <<"\n Called scheduleHelloPacketEvent function ------- " <<endl;
	scheduler::schedule(ndn::time::seconds(seconds),bind(&ControllerApp::sendScheduledHelloInterest, this, seconds));
}

void
ControllerApp::OnTimeout(uint32_t sequenceNumber)
{
	cout << "\n OnTimeout Called " << endl;
}

void
ControllerApp::expressInterest(const Name& interestName, uint32_t seconds)
{
	cout<< "\n Expressing Hello Interest :" << interestName << endl;
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
	cout <<"\n Called sendScheduledHelloInterest function ------- " <<endl;
	cout << "\n Source node for Hello packets->  " << Names::FindName(GetNode())<<endl;
	cout << "\n Printing list value before sending Hello packets " <<endl;
	//m_gb_adList = CollectLinks();
	m_gb_adList.writeLog();
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
	    /*
	    else {
	      registerPrefixes((*it).getName(), (*it).getConnectingFaceUri(),
	                       (*it).getLinkCost(), ndn::time::milliseconds::max());
	    }*/
	  }
	cout <<"\n Numeber of times this function called ->  " << counter << endl;
	cout << "\n Printing list value after sending Hello packets " <<endl;
	m_gb_adList.writeLog();
	scheduleHelloPacketEvent(m_conf.getInfoInterestInterval());
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
				NS_ASSERT_MSG (node1 != 0, node->GetSourceNode() << "is not a Node");

				Ptr<L3Protocol> ndn1 = node1->GetObject<L3Protocol> ();
				NS_ASSERT_MSG (ndn1 != 0, "Ndn protocol hasn't been installed on a node, please install it first");

				shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace> (ndn1->getFaceById(atoi(fields[n+1].c_str())));
				node->AddIncidency(face, otherNode, atoi(fields[n+2].c_str()));
				//my_graph.add_incidency(node,otherNode,atof(fields[n+2].c_str()));
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
		  dPacket->setFreshnessPeriod(ndn::time::milliseconds(6000));
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
			CalculateKPathYanAlgorithm(3); // Calling Yan's K path algorithm.
			StartSendingPathToNode(); // Start seding packets to individual nodes.
		}
	}
	else if(strRequestType.compare("req_update") == 0)
	{
	   // We got the data packet for updating the routes.
	  NdnControllerString strControllerUpdatedData = NdnControllerString(msg);
	  std::string strSourceNodeUpdate =	strControllerUpdatedData.GetSourceNode();
	  std::cout << "\n Received data packet for updating information from node  ->  " << strSourceNodeUpdate << "\t  information is -> " << msg << endl;
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
