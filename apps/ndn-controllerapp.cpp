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

#include <boost/ref.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/algorithm/string.hpp>

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
					"Prefix, for which producer has the data", StringValue("/"),
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

std::string ControllerApp::extractNodeName(std::string strPacketName) {
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strPacketName,
			boost::algorithm::is_any_of("/"));
	//for (size_t n = 0; n < fields.size(); n++)
	//	std::cout << fields[n] << "\"\n";
	//cout << endl;

	return fields[2];
}

std::string ControllerApp::extractNodeRequestType(std::string strPrefixName) {
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strPrefixName,
			boost::algorithm::is_any_of("/"));
	//for (size_t n = 0; n < fields.size(); n++)
	//	std::cout << fields[n] << "\"\n";
	//cout << endl;
	return fields[3];
}


 void ControllerApp::extractNodeLinkInfo(std::string strNodeLinkInfo) {
	std::vector<std::string> fields;
	boost::algorithm::split(fields, strNodeLinkInfo,
				boost::algorithm::is_any_of(","));

	for (size_t n = 0; n < fields.size(); n+=4)
	{
		Ptr<Node> node1 = Names::Find<Node> (fields[n]);
		NS_ASSERT_MSG (node1 != 0, fields[n] << "is not a Node");

		Ptr<L3Protocol> ndn1 = node1->GetObject<L3Protocol> ();

		NS_ASSERT_MSG (ndn1 != 0, "Ndn protocol hasn't been installed on a node, please install it first");

		shared_ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace> (ndn1->getFaceById(atoi(fields[n+1].c_str())));
		//Ptr<NetDeviceFace> face = dynamic_pointer_cast<NetDeviceFace>(atoi(fields[n+1].c_str())));

		Ptr<Node> node2 = Names::Find<Node> (fields[n+2]);
		NS_ASSERT_MSG (node1 != 0, fields[n] << "is not a Node");

		cout << "\n 1: " << fields[n] << " 2: " <<  fields[n+1] << " 3: " << fields[n+2] << " 4: " << fields[n+3] << endl;

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

void ControllerApp::sendDataPacket(std::shared_ptr<const Interest> interest){

}


void ControllerApp::OnInterest(std::shared_ptr<const Interest> interest) {
	App::OnInterest(interest); // tracing inside
	NS_LOG_FUNCTION(this << interest);

	std::cout << "\n";
	std::cout << "CentralizedControllerApp: Received interest packet -> " << interest->getName() << std::endl;

	if (!m_active)
		return;


	std::string strPrefix;
	std::string strRequestType = extractNodeRequestType(interest->getName().toUri());
	std::string strInterestNodePrefix = extractNodeName(interest->getName().toUri());

	if (strRequestType.compare("req_route") == 0)
	{
		strPrefix = "/" + strInterestNodePrefix + "/controller" + "/req_route";
		std::cout << "CentralizedControllerApp: Sending interest packet to  " << strInterestNodePrefix << "  with acknowledge interest -> " << strPrefix << std::endl;
		sendInterestPacket(strPrefix);
	}
	else if(strRequestType.compare("res_route") == 0)
	{
		std::cout << "CentralizedControllerApp: Sending data packet to  " << strInterestNodePrefix << "  with calculated distance "<< std::endl;
		strPrefix = "/" + strInterestNodePrefix + "/controller" + "/res_route";
		sendDataPacket(interest);
	}
	else{
		strPrefix = "/";
		sendInterestPacket(strPrefix);
	}

}


void ControllerApp::OnData(std::shared_ptr<const Data> contentObject) {
	App::OnData(contentObject); // tracing inside
	NS_LOG_FUNCTION(this << contentObject);
	if (!m_active)
    	return;
	NS_LOG_INFO ("Received content object: " << boost::cref(*contentObject));
	std::cout << "CentralizedControllerApp: Received Data packet -> "
			<< contentObject->getName() << std::endl;
	std::string msg(reinterpret_cast<const char*>(contentObject->getContent().value()),
			contentObject->getContent().value_size());
	//std::cout << msg<< std::endl;

	cout << "Packet Data ->  "<< msg <<endl;
	extractNodeLinkInfo(msg);
//	if (extractNodeLinkInfo(msg))
//	{
		//calculate the route here...
		std:string strInterestPrefix = "/" + extractNodeName(contentObject->getName().toUri()) + "/controller" + "/res_route";
		sendInterestPacket(strInterestPrefix);
//	}

	std::cout << "\n" << endl;
	std::cout<< "####################################### Stop Three Way Communication with Controller ###############################################################"<< std::endl;
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