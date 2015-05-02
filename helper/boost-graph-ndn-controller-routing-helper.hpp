#ifndef BOOST_GRAPH_NDN_CONTROLLER_ROUTING_HELPER_H
#define BOOST_GRAPH_NDN_CONTROLLER_ROUTING_HELPER_H

/// @cond include_hidden

#include "ns3/ndnSIM/model/ndn-common.hpp"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/ref.hpp>

#include "ns3/ndnSIM/model/ndn-face.hpp"

#include "model/ndn-controller-router.hpp"
#include "helper/controller-node-list.hpp"

#include <list>
#include <map>

namespace boost {

class NdnControllerRouterGraph {
public:
  typedef ns3::Ptr<ns3::ndn::ControllerRouter> Vertice;
  typedef uint16_t edge_property_type;
  typedef uint32_t vertex_property_type;

  NdnControllerRouterGraph()
  {
	  for (ns3::ndn::ControllerNodeList::Iterator j = ns3::ndn::ControllerNodeList::Begin (); j != ns3::ndn::ControllerNodeList::End (); j++)
	  {
		  	  if((*j)->GetId() != 0)
		  	  {
		  		  ns3::Ptr<ns3::ndn::ControllerRouter> gr = (*j);
		  		  //(*j)->AddMultiPathIncidencies((*j)->GetIncidencies());
		  		  /*
		  		  std::cout << "Node Name -> " << (*j)->GetSourceNode() << std::endl;
		  		  std::cout << "Link Size -> " << (*j)->GetIncidencies().size() << std::endl;
		  		  std::cout << "Prefix Size -> " << (*j)->GetLocalPrefixes().size() << std::endl;
		  		  for (const auto& prefix : (*j)->GetLocalPrefixes())
		  		  {
		  			  std::cout << "prefix -> " << prefix->toUri().c_str() << "\n";
		  		  }*/
		  		  m_vertices.push_back(gr);
		  	  }
	  }
  }
  const std::list<Vertice>&
  GetVertices() const
  {
    return m_vertices;
  }

public:
  std::list<Vertice> m_vertices;
};

class ndn_controller_router_graph_category : public virtual vertex_list_graph_tag,
                                         public virtual incidence_graph_tag {
};

template<>
struct graph_traits<NdnControllerRouterGraph> {
  // Graph concept
  typedef NdnControllerRouterGraph::Vertice vertex_descriptor;
  typedef ns3::ndn::ControllerRouter::Incidency edge_descriptor;
  typedef directed_tag directed_category;
  typedef disallow_parallel_edge_tag edge_parallel_category;
  typedef ndn_controller_router_graph_category traversal_category;

  // VertexList concept
  typedef std::list<vertex_descriptor>::const_iterator vertex_iterator;
  typedef size_t vertices_size_type;

  // AdjacencyGraph concept
  //typedef ns3::ndn::ControllerRouter::IncidencyList::iterator out_edge_iterator;
  typedef ns3::ndn::ControllerRouter::MultiPathIncidencyList::iterator out_edge_iterator;
  typedef size_t degree_size_type;

  // typedef size_t edges_size_type;
};

} // namespace boost

namespace boost {

inline graph_traits<NdnControllerRouterGraph>::vertex_descriptor
source(graph_traits<NdnControllerRouterGraph>::edge_descriptor e, const NdnControllerRouterGraph& g)
{
  return std::get<0>(e);
}

inline graph_traits<NdnControllerRouterGraph>::vertex_descriptor
target(graph_traits<NdnControllerRouterGraph>::edge_descriptor e, const NdnControllerRouterGraph& g)
{
  return std::get<2>(e);
}

inline std::pair<graph_traits<NdnControllerRouterGraph>::vertex_iterator,
                 graph_traits<NdnControllerRouterGraph>::vertex_iterator>
vertices(const NdnControllerRouterGraph& g)
{
  return make_pair(g.GetVertices().begin(), g.GetVertices().end());
}

inline graph_traits<NdnControllerRouterGraph>::vertices_size_type
num_vertices(const NdnControllerRouterGraph& g)
{
  return g.GetVertices().size();
}

inline std::pair<graph_traits<NdnControllerRouterGraph>::out_edge_iterator,
                 graph_traits<NdnControllerRouterGraph>::out_edge_iterator>
out_edges(graph_traits<NdnControllerRouterGraph>::vertex_descriptor u, const NdnControllerRouterGraph& g)
{
  return std::make_pair(u->GetMultiPathIncidencies().begin(), u->GetMultiPathIncidencies().end());
}

inline graph_traits<NdnControllerRouterGraph>::degree_size_type
out_degree(graph_traits<NdnControllerRouterGraph>::vertex_descriptor u, const NdnControllerRouterGraph& g)
{
	return u->GetMultiPathIncidencies().size();
}

//////////////////////////////////////////////////////////////
// Property maps

struct EdgeWeights {
  EdgeWeights(const NdnControllerRouterGraph& graph)
    : m_graph(graph)
  {
  }

private:
  const NdnControllerRouterGraph& m_graph;
};

struct VertexIds {
  VertexIds(const NdnControllerRouterGraph& graph)
    : m_graph(graph)
  {
  }

private:
  const NdnControllerRouterGraph& m_graph;
};

template<>
struct property_map<NdnControllerRouterGraph, edge_weight_t> {
  typedef const EdgeWeights const_type;
  typedef EdgeWeights type;
};

template<>
struct property_map<NdnControllerRouterGraph, vertex_index_t> {
  typedef const VertexIds const_type;
  typedef VertexIds type;
};

template<>
struct property_traits<EdgeWeights> {
  // Metric property map
  typedef std::tuple<std::shared_ptr<nfd::Face>, uint16_t, double> value_type;
  typedef std::tuple<std::shared_ptr<nfd::Face>, uint16_t, double> reference;
  typedef ns3::ndn::ControllerRouter::Incidency key_type;
  typedef readable_property_map_tag category;
};

const property_traits<EdgeWeights>::value_type WeightZero(nullptr, 0, 0.0);
const property_traits<EdgeWeights>::value_type
  WeightInf(nullptr, std::numeric_limits<uint16_t>::max(), 0.0);

struct WeightCompare : public std::binary_function<property_traits<EdgeWeights>::reference,
                                                   property_traits<EdgeWeights>::reference, bool> {
  bool
  operator()(std::tuple<std::shared_ptr<nfd::Face>, uint32_t, double> a,
             std::tuple<std::shared_ptr<nfd::Face>, uint32_t, double> b) const
  {
    return std::get<1>(a) < std::get<1>(b);
  }

  bool
  operator()(property_traits<EdgeWeights>::reference a, uint32_t b) const
  {
    return std::get<1>(a) < b;
  }

  bool
  operator()(uint32_t a, uint32_t b) const
  {
    return a < b;
  }
};

struct WeightCombine
  : public std::binary_function<uint32_t, property_traits<EdgeWeights>::reference, uint32_t> {
  uint32_t
  operator()(uint32_t a, property_traits<EdgeWeights>::reference b) const
  {
    return a + std::get<1>(b);
  }

  std::tuple<std::shared_ptr<nfd::Face>, uint32_t, double>
  operator()(std::tuple<std::shared_ptr<nfd::Face>, uint32_t, double> a,
             property_traits<EdgeWeights>::reference b) const
  {
    if (std::get<0>(a) == nullptr)
      return std::make_tuple(std::get<0>(b), std::get<1>(a) + std::get<1>(b),
                             std::get<2>(a) + std::get<2>(b));
    else
      return std::make_tuple(std::get<0>(a), std::get<1>(a) + std::get<1>(b),
                             std::get<2>(a) + std::get<2>(b));
  }
};

template<>
struct property_traits<VertexIds> {
  // Metric property map
  typedef uint32_t value_type;
  typedef uint32_t reference;
  typedef ns3::Ptr<ns3::ndn::ControllerRouter> key_type;
  typedef readable_property_map_tag category;
};

inline EdgeWeights
get(edge_weight_t, const NdnControllerRouterGraph& g)
{
  return EdgeWeights(g);
}

inline VertexIds
get(vertex_index_t, const NdnControllerRouterGraph& g)
{
  return VertexIds(g);
}

template<class M, class K, class V>
inline void
put(reference_wrapper<M> mapp, K a, V p)
{
  mapp.get()[a] = p;
}

// void
// put (cref< std::map< ns3::Ptr<ns3::ndn::ControllerRouter>, ns3::Ptr<ns3::ndn::ControllerRouter> > > map,

inline uint32_t
get(const boost::VertexIds&, ns3::Ptr<ns3::ndn::ControllerRouter>& gr)
{
	return gr->GetId();
}

inline property_traits<EdgeWeights>::reference
get(const boost::EdgeWeights&, ns3::ndn::ControllerRouter::Incidency& edge)
{
  if (std::get<1>(edge) == 0)
    return property_traits<EdgeWeights>::reference(nullptr, 0, 0.0);
  else {
    return property_traits<EdgeWeights>::reference(std::get<1>(edge), static_cast<uint32_t>(std::get<3>(edge)),0.0);
  }
}

struct PredecessorsMap
  : public std::map<ns3::Ptr<ns3::ndn::ControllerRouter>, ns3::Ptr<ns3::ndn::ControllerRouter>> {
};

template<>
struct property_traits<reference_wrapper<PredecessorsMap>> {
  // Metric property map
  typedef ns3::Ptr<ns3::ndn::ControllerRouter> value_type;
  typedef ns3::Ptr<ns3::ndn::ControllerRouter> reference;
  typedef ns3::Ptr<ns3::ndn::ControllerRouter> key_type;
  typedef read_write_property_map_tag category;
};

struct DistancesMap : public std::map<ns3::Ptr<ns3::ndn::ControllerRouter>,
                                      std::tuple<std::shared_ptr<nfd::Face>, uint32_t, double>> {
};

template<>
struct property_traits<reference_wrapper<DistancesMap>> {
  // Metric property map
  typedef std::tuple<std::shared_ptr<nfd::Face>, uint32_t, double> value_type;
  typedef std::tuple<std::shared_ptr<nfd::Face>, uint32_t, double> reference;
  typedef ns3::Ptr<ns3::ndn::ControllerRouter> key_type;
  typedef read_write_property_map_tag category;
};

} // boost

namespace std {
template<>
class numeric_limits<std::tuple<std::shared_ptr<nfd::Face>, uint32_t, double>>
{
public:
  typedef std::tuple<std::shared_ptr<nfd::Face>, uint32_t, double> value;
  static value
  max()
  {
    return boost::WeightInf;
  }
};
}

namespace boost {

inline std::tuple<std::shared_ptr<nfd::Face>, uint32_t, double>
get(DistancesMap& map, ns3::Ptr<ns3::ndn::ControllerRouter> key)
{
  boost::DistancesMap::iterator i = map.find(key);
  if (i == map.end())
    return std::tuple<std::shared_ptr<nfd::Face>, uint32_t,
                      double>(nullptr, std::numeric_limits<uint32_t>::max(), 0.0);
  else
    return i->second;
}

} // namespace boost

/// @endcond

#endif // BOOST_GRAPH_NDN_CONTROLLER_ROUTING_HELPER_H
