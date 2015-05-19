#ifndef NDN_YANGRAPHALGORITHM_H
#define NDN_YANGRAPHALGORITHM_H

#pragma once
#include "model/ndn-controller-router.hpp"

using namespace std;

namespace ns3 {
namespace ndn {


class DijkstraShortestPathAlg
{
private: // members

	Graph* m_pDirectGraph;

	std::map<Ptr<ControllerRouter>, double> m_mpStartDistanceIndex;
	std::map<Ptr<ControllerRouter>, Ptr<ControllerRouter>> m_mpPredecessorVertex;

	std::set<int> m_stDeterminedVertices;

	std::multiset<Ptr<ControllerRouter>, WeightLess<Ptr<ControllerRouter>> > m_quCandidateVertices;

public:
	DijkstraShortestPathAlg(Graph* pGraph):m_pDirectGraph(pGraph){}
	~DijkstraShortestPathAlg(void){clear();}

	void clear();

	BasePath* get_shortest_path(Ptr<ControllerRouter> source, Ptr<ControllerRouter> sink);

	void set_predecessor_vertex(Ptr<ControllerRouter> vt1, Ptr<ControllerRouter> vt2)
	{
		m_mpPredecessorVertex[vt1] = vt2;
	}

	double get_start_distance_at(Ptr<ControllerRouter> vertex)
	{
		return m_mpStartDistanceIndex.find(vertex)->second;
	}

	void set_start_distance_at(Ptr<ControllerRouter> vertex, double weight)
	{
		m_mpStartDistanceIndex[vertex] = weight;
	}

	void get_shortest_path_flower(Ptr<ControllerRouter> root)
	{
		determine_shortest_paths(NULL, root, false);
	}

	// The following two methods are prepared for the top-k shortest paths algorithm
	BasePath* update_cost_forward(Ptr<ControllerRouter> vertex);
	void correct_cost_backward(Ptr<ControllerRouter> vertex);

protected:

	void determine_shortest_paths(Ptr<ControllerRouter> source, Ptr<ControllerRouter> sink, bool is_source2sink);

	void improve2vertex(Ptr<ControllerRouter> cur_vertex_pt, bool is_source2sink);

};


class YenTopKShortestPathsAlg
{
	Graph* m_pGraph;

	vector<BasePath*> m_vResultList;
	map<BasePath*, Ptr<ControllerRouter>> m_mpDerivationVertexIndex;
	multiset<BasePath*, WeightLess<BasePath> > m_quPathCandidates;

	Ptr<ControllerRouter> m_pSourceVertex;
	Ptr<ControllerRouter> m_pTargetVertex;

	int m_nGeneratedPathNum;

private:

	void _init();

public:

	YenTopKShortestPathsAlg(const Graph& graph)
	{
		YenTopKShortestPathsAlg(graph, NULL, NULL);
	}

	YenTopKShortestPathsAlg(const Graph& graph, Ptr<ControllerRouter> pSource, Ptr<ControllerRouter> pTarget)
		:m_pSourceVertex(pSource), m_pTargetVertex(pTarget)
	{
		m_pGraph = new Graph(graph);
		_init();
	}

	~YenTopKShortestPathsAlg(void){clear();}

	void clear();
	bool has_next();
	BasePath* next();

	BasePath* get_shortest_path(Ptr<ControllerRouter> pSource, Ptr<ControllerRouter> pTarget);
	void get_shortest_paths(Ptr<ControllerRouter> pSource, Ptr<ControllerRouter> pTarget, int top_k,
		vector<BasePath*>&);
};

}
}

#endif /* NDN_YANGRAPHALGORITHM_H */
