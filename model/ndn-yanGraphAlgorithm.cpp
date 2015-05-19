#include <set>
#include <map>
#include <vector>
#include "ndn-yanGraph.hpp"
#include "ndn-yanGraphAlgorithm.hpp"

namespace ns3 {
namespace ndn {

BasePath* DijkstraShortestPathAlg::get_shortest_path( Ptr<ControllerRouter> source, Ptr<ControllerRouter> sink )
{
	determine_shortest_paths(source, sink, true);

	std::vector<Ptr<ControllerRouter>> vertex_list;
	std::map<Ptr<ControllerRouter>, double>::const_iterator pos =
		m_mpStartDistanceIndex.find(sink);
	double weight = pos != m_mpStartDistanceIndex.end() ? pos->second : Graph::DISCONNECT;

	if (weight < Graph::DISCONNECT)
	{
		Ptr<ControllerRouter> cur_vertex_pt = sink;
		do
		{
			vertex_list.insert(vertex_list.begin(), cur_vertex_pt);

			std::map<Ptr<ControllerRouter>, Ptr<ControllerRouter>>::const_iterator pre_pos =
				m_mpPredecessorVertex.find(cur_vertex_pt);

			if (pre_pos == m_mpPredecessorVertex.end()) break;

			cur_vertex_pt = pre_pos->second;

		} while (cur_vertex_pt != source);

		vertex_list.insert(vertex_list.begin(), source);
	}
	return new BasePath(vertex_list, weight);
}

void DijkstraShortestPathAlg::determine_shortest_paths( Ptr<ControllerRouter> source, Ptr<ControllerRouter> sink, bool is_source2sink )
{
	//1. clear the intermediate variables
	clear();

	//2. initiate the local variables
	Ptr<ControllerRouter> end_vertex = is_source2sink ? sink : source;
	Ptr<ControllerRouter> start_vertex = is_source2sink ? source : sink;
	m_mpStartDistanceIndex[start_vertex] = 0;
	start_vertex->Weight(0);
	m_quCandidateVertices.insert(start_vertex);

	//3. start searching for the shortest path
	while (!m_quCandidateVertices.empty())
	{
		multiset<Ptr<ControllerRouter>, WeightLess<ControllerRouter> >::const_iterator pos = m_quCandidateVertices.begin();

		Ptr<ControllerRouter> cur_vertex_pt = *pos; //m_quCandidateVertices.top();
		m_quCandidateVertices.erase(pos);

		if (cur_vertex_pt == end_vertex) break;

		m_stDeterminedVertices.insert(cur_vertex_pt->getID());

		improve2vertex(cur_vertex_pt, is_source2sink);
	}
}

void DijkstraShortestPathAlg::improve2vertex( Ptr<ControllerRouter> cur_vertex_pt, bool is_source2sink )
{
	// 1. get the neighboring vertices
	set<Ptr<ControllerRouter>>* neighbor_vertex_list_pt = new set<Ptr<ControllerRouter>>();

	if(is_source2sink)
	{
		m_pDirectGraph->get_adjacent_vertices(cur_vertex_pt, *neighbor_vertex_list_pt);
	}else
	{
		m_pDirectGraph->get_precedent_vertices(cur_vertex_pt, *neighbor_vertex_list_pt);
	}

	// 2. update the distance passing on the current vertex
	for(set<Ptr<ControllerRouter>>::iterator cur_neighbor_pos=neighbor_vertex_list_pt->begin();
		cur_neighbor_pos!=neighbor_vertex_list_pt->end(); ++cur_neighbor_pos)
	{
		//2.1 skip if it has been visited before
		if (m_stDeterminedVertices.find((*cur_neighbor_pos)->getID())!=m_stDeterminedVertices.end())
		{
			continue;
		}

		//2.2 calculate the distance
		map<Ptr<ControllerRouter>, double>::const_iterator cur_pos = m_mpStartDistanceIndex.find(cur_vertex_pt);
		double distance =  cur_pos != m_mpStartDistanceIndex.end() ? cur_pos->second : Graph::DISCONNECT;

		distance += is_source2sink ? m_pDirectGraph->get_edge_weight(cur_vertex_pt, *cur_neighbor_pos) :
			m_pDirectGraph->get_edge_weight(*cur_neighbor_pos, cur_vertex_pt);

		//2.3 update the distance if necessary
		cur_pos = m_mpStartDistanceIndex.find(*cur_neighbor_pos);
		if (cur_pos == m_mpStartDistanceIndex.end() || cur_pos->second > distance)
		{
			m_mpStartDistanceIndex[*cur_neighbor_pos] = distance;
			m_mpPredecessorVertex[*cur_neighbor_pos] = cur_vertex_pt;

			(*cur_neighbor_pos)->Weight(distance);

			multiset<Ptr<ControllerRouter>, WeightLess<ControllerRouter> >::const_iterator pos = m_quCandidateVertices.begin();
			for(; pos != m_quCandidateVertices.end(); ++pos)
			{
				if ((*pos)->getID() == (*cur_neighbor_pos)->getID())
				{
					break;
				}
			}
			if(pos != m_quCandidateVertices.end())
			{
				m_quCandidateVertices.erase(pos);
			}
			m_quCandidateVertices.insert(*cur_neighbor_pos);
		}
	}
}

void DijkstraShortestPathAlg::clear()
{
	m_stDeterminedVertices.clear();
	m_mpPredecessorVertex.clear();
	m_mpStartDistanceIndex.clear();
	m_quCandidateVertices.clear();
}

BasePath* DijkstraShortestPathAlg::update_cost_forward( Ptr<ControllerRouter> vertex )
{
	double cost = Graph::DISCONNECT;

 	// 1. get the set of successors of the input vertex
	set<Ptr<ControllerRouter>>* adj_vertex_set = new set<Ptr<ControllerRouter>>();
	m_pDirectGraph->get_adjacent_vertices(vertex, *adj_vertex_set);

 	// 2. make sure the input vertex exists in the index
	map<Ptr<ControllerRouter>, double>::iterator pos4vertexInStartDistIndex = m_mpStartDistanceIndex.find(vertex);
	if(pos4vertexInStartDistIndex == m_mpStartDistanceIndex.end())
 	{
		pos4vertexInStartDistIndex =
			(m_mpStartDistanceIndex.insert(make_pair(vertex, Graph::DISCONNECT))).first;
 	}

 	// 3. update the distance from the root to the input vertex if necessary
 	for(set<Ptr<ControllerRouter>>::const_iterator pos=adj_vertex_set->begin(); pos!=adj_vertex_set->end();++pos)
 	{
 		// 3.1 get the distance from the root to one successor of the input vertex
		map<Ptr<ControllerRouter>, double>::const_iterator cur_vertex_pos = m_mpStartDistanceIndex.find(*pos);
		double distance = cur_vertex_pos == m_mpStartDistanceIndex.end() ?
			Graph::DISCONNECT : cur_vertex_pos->second;

 		// 3.2 calculate the distance from the root to the input vertex
		distance += m_pDirectGraph->get_edge_weight(vertex, *pos);

 		// 3.3 update the distance if necessary
		double cost_of_vertex = pos4vertexInStartDistIndex->second;
 		if(cost_of_vertex > distance)
 		{
			m_mpStartDistanceIndex[vertex] = distance;
			m_mpPredecessorVertex[vertex] = cur_vertex_pos->first;
 			cost = distance;
 		}
 	}

 	// 4. create the sub_path if exists
	BasePath* sub_path = NULL;
	if(cost < Graph::DISCONNECT)
 	{
		vector<Ptr<ControllerRouter>> vertex_list;
		vertex_list.push_back(vertex);

		map<Ptr<ControllerRouter>, Ptr<ControllerRouter>>::const_iterator pos4PredVertexMap =
			m_mpPredecessorVertex.find(vertex);

		while(pos4PredVertexMap != m_mpPredecessorVertex.end())
		{
			Ptr<ControllerRouter> pred_vertex_pt = pos4PredVertexMap->second;
			vertex_list.push_back(pred_vertex_pt);
			pos4PredVertexMap = m_mpPredecessorVertex.find(pred_vertex_pt);
		}

		sub_path = new BasePath(vertex_list, cost);
 	}
 	return sub_path;
}

void DijkstraShortestPathAlg::correct_cost_backward( Ptr<ControllerRouter> vertex )
{
 	// 1. initialize the list of vertex to be updated
	vector<Ptr<ControllerRouter>> vertex_pt_list;
	vertex_pt_list.push_back(vertex);

	// 2. update the cost of relevant precedents of the input vertex
	while(!vertex_pt_list.empty())
 	{
		Ptr<ControllerRouter> cur_vertex_pt = *(vertex_pt_list.begin());
		vertex_pt_list.erase(vertex_pt_list.begin());

 		double cost_of_cur_vertex = m_mpStartDistanceIndex[cur_vertex_pt];

		set<Ptr<ControllerRouter>> pre_vertex_set;
		m_pDirectGraph->get_precedent_vertices(cur_vertex_pt, pre_vertex_set);
		for(set<Ptr<ControllerRouter>>::const_iterator pos=pre_vertex_set.begin(); pos!=pre_vertex_set.end();++pos)
		{
			map<Ptr<ControllerRouter>,double>::const_iterator pos4StartDistIndexMap =
				m_mpStartDistanceIndex.find(*pos);
			double cost_of_pre_vertex = m_mpStartDistanceIndex.end() == pos4StartDistIndexMap ?
				Graph::DISCONNECT : pos4StartDistIndexMap->second;

			double fresh_cost = cost_of_cur_vertex + m_pDirectGraph->get_edge_weight(*pos, cur_vertex_pt);
			if(cost_of_pre_vertex > fresh_cost)
			{
				m_mpStartDistanceIndex[*pos] = fresh_cost;
				m_mpPredecessorVertex[*pos] = cur_vertex_pt;
				vertex_pt_list.push_back(*pos);
			}
		}
	}
}

void YenTopKShortestPathsAlg::clear()
{
	m_nGeneratedPathNum = 0;
	m_mpDerivationVertexIndex.clear();
	m_vResultList.clear();
	m_quPathCandidates.clear();
}

void YenTopKShortestPathsAlg::_init()
{
	clear();
	if (m_pSourceVertex != NULL && m_pTargetVertex != NULL)
	{
		BasePath* pShortestPath = get_shortest_path(m_pSourceVertex, m_pTargetVertex);
		if (pShortestPath != NULL && pShortestPath->length() > 1)
		{
			m_quPathCandidates.insert(pShortestPath);
			m_mpDerivationVertexIndex[pShortestPath] = m_pSourceVertex;
		}
	}
}

BasePath* YenTopKShortestPathsAlg::get_shortest_path( Ptr<ControllerRouter> pSource, Ptr<ControllerRouter> pTarget )
{
	DijkstraShortestPathAlg dijkstra_alg(m_pGraph);
	return dijkstra_alg.get_shortest_path(pSource, pTarget);
}

bool YenTopKShortestPathsAlg::has_next()
{
	return !m_quPathCandidates.empty();
}

BasePath* YenTopKShortestPathsAlg::next()
{
	//1. Prepare for removing vertices and arcs
	BasePath* cur_path = *(m_quPathCandidates.begin());//m_quPathCandidates.top();

	//m_quPathCandidates.pop();
	m_quPathCandidates.erase(m_quPathCandidates.begin());
	m_vResultList.push_back(cur_path);

	int count = m_vResultList.size();

	Ptr<ControllerRouter> cur_derivation_pt = m_mpDerivationVertexIndex.find(cur_path)->second;
	vector<Ptr<ControllerRouter>> sub_path_of_derivation_pt;
	cur_path->SubPath(sub_path_of_derivation_pt, cur_derivation_pt);
	int sub_path_length = sub_path_of_derivation_pt.size();

	//2. Remove the vertices and arcs in the graph
	for (int i=0; i<count-1; ++i)
	{
		BasePath* cur_result_path = m_vResultList.at(i);
		vector<Ptr<ControllerRouter>> cur_result_sub_path_of_derivation_pt;

		if (!cur_result_path->SubPath(cur_result_sub_path_of_derivation_pt, cur_derivation_pt)) continue;

		if (sub_path_length != cur_result_sub_path_of_derivation_pt.size()) continue;

		bool is_equal = true;
		for (int i=0; i<sub_path_length; ++i)
		{
			if (sub_path_of_derivation_pt.at(i) != cur_result_sub_path_of_derivation_pt.at(i))
			{
				is_equal = false;
				break;
			}
		}
		if (!is_equal) continue;

		//
		Ptr<ControllerRouter> cur_succ_vertex = cur_result_path->GetVertex(sub_path_length+1);
		m_pGraph->remove_edge(make_pair(cur_derivation_pt->getID(), cur_succ_vertex->getID()));
	}

	//2.1 remove vertices and edges along the current result
	int path_length = cur_path->length();
	for(int i=0; i<path_length-1; ++i)
	{
		m_pGraph->remove_vertex(cur_path->GetVertex(i)->getID());
		m_pGraph->remove_edge(make_pair(
			cur_path->GetVertex(i)->getID(), cur_path->GetVertex(i+1)->getID()));
	}

	//3. Calculate the shortest tree rooted at target vertex in the graph
	DijkstraShortestPathAlg reverse_tree(m_pGraph);
	reverse_tree.get_shortest_path_flower(m_pTargetVertex);

	//4. Recover the deleted vertices and update the cost and identify the new candidates results
	bool is_done = false;
	for(int i=path_length-2; i>=0 && !is_done; --i)
	{
		//4.1 Get the vertex to be recovered
		Ptr<ControllerRouter> cur_recover_vertex = cur_path->GetVertex(i);
		m_pGraph->recover_removed_vertex(cur_recover_vertex->getID());

		//4.2 Check if we should stop continuing in the next iteration
		if (cur_recover_vertex->getID() == cur_derivation_pt->getID())
		{
			is_done = true;
		}

		//4.3 Calculate cost using forward star form
		BasePath* sub_path = reverse_tree.update_cost_forward(cur_recover_vertex);

		//4.4 Get one candidate result if possible
		if (sub_path != NULL)
		{
			++m_nGeneratedPathNum;

			//4.4.1 Get the prefix from the concerned path
			double cost = 0;
			reverse_tree.correct_cost_backward(cur_recover_vertex);

			vector<Ptr<ControllerRouter>> pre_path_list;
			for (int j=0; j<path_length; ++j)
			{
				Ptr<ControllerRouter> cur_vertex = cur_path->GetVertex(j);
				if (cur_vertex->getID() == cur_recover_vertex->getID())
				{
					//j = path_length;
					break;
				}else
				{
					cost += m_pGraph->get_original_edge_weight(
						cur_path->GetVertex(j), cur_path->GetVertex(1+j));
					pre_path_list.push_back(cur_vertex);
				}
			}
			//
			for (int j=0; j<sub_path->length(); ++j)
			{
				pre_path_list.push_back(sub_path->GetVertex(j));
			}

			//4.4.2 Compose a candidate
			sub_path = new Path(pre_path_list, cost+sub_path->Weight());

			//4.4.3 Put it in the candidate pool if new
			if (m_mpDerivationVertexIndex.find(sub_path) == m_mpDerivationVertexIndex.end())
			{
				m_quPathCandidates.insert(sub_path);
				m_mpDerivationVertexIndex[sub_path] = cur_recover_vertex;
			}
		}

		//4.5 Restore the edge
		Ptr<ControllerRouter> succ_vertex = cur_path->GetVertex(i+1);
		m_pGraph->recover_removed_edge(make_pair(cur_recover_vertex->getID(), succ_vertex->getID()));

		//4.6 Update cost if necessary
		double cost_1 = m_pGraph->get_edge_weight(cur_recover_vertex, succ_vertex)
			+ reverse_tree.get_start_distance_at(succ_vertex);

		if (reverse_tree.get_start_distance_at(cur_recover_vertex) > cost_1)
		{
			reverse_tree.set_start_distance_at(cur_recover_vertex, cost_1);
			reverse_tree.set_predecessor_vertex(cur_recover_vertex, succ_vertex);
			reverse_tree.correct_cost_backward(cur_recover_vertex);
		}
	}

	//5. Restore everything
	m_pGraph->recover_removed_edges();
	m_pGraph->recover_removed_vertices();

	return cur_path;
}

void YenTopKShortestPathsAlg::get_shortest_paths( Ptr<ControllerRouter> pSource,
	Ptr<ControllerRouter> pTarget, int top_k, vector<BasePath*>& result_list)
{
	m_pSourceVertex = pSource;
	m_pTargetVertex = pTarget;

	_init();
	int count = 0;
	while (has_next() && count < top_k)
	{
		next();
		++count;
	}

	result_list.assign(m_vResultList.begin(),m_vResultList.end());
}

}
}
