#include "ndn-conf-parameter.hpp"

using namespace std;

namespace ns3 {
namespace ndn {

void ConfParameter::writeLog() {
	cout << "Router Name: " << m_routerName << endl;
	cout << "Site Name: " << m_siteName << endl;
	cout << "Network: " << m_network << endl;
	cout << "Router Prefix: " << m_routerPrefix << endl;
	cout << "ChronoSync sync Prefix: " << m_chronosyncPrefix << endl;
	cout << "ChronoSync LSA prefix: " << m_lsaPrefix << endl;
	cout << "Hello Interest retry number: " << m_interestRetryNumber<<endl;
	cout << "Hello Interest resend second: " << m_interestResendTime << endl;
	cout << "Info Interest interval: " << m_infoInterestInterval << endl;
	cout << "LSA refresh time: " << m_lsaRefreshTime << endl;
	cout << "LSA Interest lifetime: " << getLsaInterestLifetime() << endl;
	cout << "Router dead interval: " << getRouterDeadInterval() << endl;
	cout << "Max Faces Per Prefix: " << m_maxFacesPerPrefix << endl;
	cout << "Hyperbolic Routing: " << m_hyperbolicState << endl;
	cout << "Hyp R: " << m_corR << endl;
	cout << "Hyp theta: " << m_corTheta << endl;
	cout << "Log Directory: " << m_logDir << endl;
	cout << "Seq Directory: " << m_seqFileDir << endl;

	// Event Intervals
	cout << "Adjacency LSA build interval:  " << m_adjLsaBuildInterval << endl;
	cout << "First Hello Interest interval: " << m_firstHelloInterval << endl;
	cout << "Routing calculation interval:  " << m_routingCalcInterval << endl;
}

} // namespace ns3
} // namespace ndn
