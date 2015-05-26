#ifndef NDN_YANGRAPH_H
#define NDN_YANGRAPH_H

#pragma once
#include <iostream>
#include "model/ndn-controller-router.hpp"

namespace ns3 {
namespace ndn {

using namespace std;


/*
template<class T>
class WeightGreater
{
public:
	// Determine priority.
	bool operator()(const T& a, const T& b) const
	{
		return a.Weight() > b.Weight();
	}

	bool operator()(const T* a, const T* b) const
	{
		return a->Weight() > b->Weight();
	}
};


template<class T>
class WeightLess
{
public:
	// Determine priority.
	bool operator()(const T& a, const T& b) const
	{
		return a.Weight() < b.Weight();
	}

	bool operator()(const T* a, const T* b) const
	{
		return a->Weight() < b->Weight();
	}
};

*/

template<class T>
class WeightGreater
{
public:
	// Determine priority.
	bool operator()(const T& a, const T& b) const
	{
		//return a.Weight() > b.Weight();
		return a->Weight() > b->Weight();
	}

	bool operator()(const T* a, const T* b) const
	{
		return a->Weight() > b->Weight();
	}

};


template<class T>
class WeightLess
{
public:
	// Determine priority.
	bool operator()(const T& a, const T& b) const
	{
		//return a.Weight() < b.Weight();
		return a->Weight() > b->Weight();
	}

	bool operator()(const T* a, const T* b) const
	{
		return a->Weight() < b->Weight();
	}

};


//////////////////////////////////////////////////////////////////////////
// A class for the object deletion
//////////////////////////////////////////////////////////////////////////
template<class T>
class DeleteFunc
{
public:
	void operator()(const T& it) const
	{
		delete it;
	}

	void operator()(const T*& it) const
	{
		delete it;
	}

	void operator()(const T* it) const
	{
		delete it;
	}
};

#if 0

/**************************************************************************
*  BaseVertex
*  <TODO: insert class description here>
*
*
*  @remarks <TODO: insert remarks here>
*
*  @author Yan Qi @date 6/6/2010
**************************************************************************/
class BaseVertex
{
	int m_nID;
	double m_dWeight;

public:

	int getID() const { return m_nID; }
	void setID(int ID_) { m_nID = ID_; }

	double Weight() const { return m_dWeight; }
	void Weight(double val) { m_dWeight = val; }

	void PrintOut(std::ostream& out_stream)
	{
		//out_stream << m_nID;
		std::cout << m_nID;
	}
};

#endif

/**************************************************************************
*  BasePath
*  <TODO: insert class description here>
*
*
*  @remarks <TODO: insert remarks here>
*
*  @author Yan Qi @date 6/6/2010
**************************************************************************/
class BasePath
{
protected:

	int m_nLength;
	double m_dWeight;
	std::vector<Ptr<ControllerRouter>> m_vtVertexList;

public:
	BasePath(const std::vector<Ptr<ControllerRouter>>& vertex_list, double weight)
		:m_dWeight(weight)
	{
		m_vtVertexList.assign(vertex_list.begin(), vertex_list.end());
		m_nLength = m_vtVertexList.size();
	}
	~BasePath(void){}

	double Weight() const { return m_dWeight; }
	void Weight(double val) { m_dWeight = val; }

	int length() { return m_nLength; }

	Ptr<ControllerRouter> GetVertex(int i)
	{
		return m_vtVertexList.at(i);
	}

	bool SubPath(std::vector<Ptr<ControllerRouter>>& sub_path, Ptr<ControllerRouter> ending_vertex_pt)
	{

		for (std::vector<Ptr<ControllerRouter>>::const_iterator pos = m_vtVertexList.begin();
			pos != m_vtVertexList.end(); ++pos)
		{
			if (*pos != ending_vertex_pt)
			{
				sub_path.push_back(*pos);
			}else
			{
				//break;
				return true;
			}
		}

		return false;
	}

	// display the content
	void PrintOut(std::ostream& out_stream) const
	{
		//out_stream << "Cost: " << m_dWeight << " Length: " << m_vtVertexList.size() << std::endl;
		std::cout << "Cost: " << m_dWeight << " Length: " << m_vtVertexList.size() << std::endl;
		for(std::vector<Ptr<ControllerRouter>>::const_iterator pos=m_vtVertexList.begin(); pos!=m_vtVertexList.end();++pos)
		{
			(*pos)->PrintOut(out_stream);
			//out_stream << "->";
			std::cout << "->";
		}
		//out_stream << std::endl <<  "*********************************************" << std::endl;
		std::cout << std::endl <<  "*********************************************" << std::endl;
	}


	Ptr<ControllerRouter> getNextNode()
	{
		if(m_vtVertexList.size()>=2)
		{
			return m_vtVertexList[1];
		}
		return NULL;
	}

};

class Path : public BasePath
{
public:

	Path(const std::vector<Ptr<ControllerRouter>>& vertex_list, double weight):BasePath(vertex_list,weight){}

	// display the content
	void PrintOut(std::ostream& out_stream) const
	{
		//out_stream << "Cost: " << m_dWeight << " Length: " << m_vtVertexList.size() << std::endl;
		std::cout << "Cost: " << m_dWeight << " Length: " << m_vtVertexList.size() << std::endl;
		for(std::vector<Ptr<ControllerRouter>>::const_iterator pos=m_vtVertexList.begin(); pos!=m_vtVertexList.end();++pos)
		{
			//out_stream << (*pos)->getID() << " ";
			std::cout << (*pos)->getID() << " ";
		}
		//out_stream << std::endl <<  "*********************************************" << std::endl;
		std::cout << std::endl <<  "*********************************************" << std::endl;
	}
};

class Graph
{
public: // members

	const static double DISCONNECT;

	typedef set<Ptr<ControllerRouter>>::iterator VertexPtSetIterator;
	typedef map<Ptr<ControllerRouter>, set<Ptr<ControllerRouter>>*>::iterator BaseVertexPt2SetMapIterator;

protected: // members

	// Basic information
	map<Ptr<ControllerRouter>, set<Ptr<ControllerRouter>>*> m_mpFanoutVertices;
	map<Ptr<ControllerRouter>, set<Ptr<ControllerRouter>>*> m_mpFaninVertices;
	map<int, double> m_mpEdgeCodeWeight;
	vector<Ptr<ControllerRouter>> m_vtVertices;
	int m_nEdgeNum;
	int m_nVertexNum;

	map<int, Ptr<ControllerRouter>> m_mpVertexIndex;

	// Members for graph modification
	set<int> m_stRemovedVertexIds;
	set<pair<int,int> > m_stRemovedEdge;

public:

	// Constructors and Destructor

	Graph(int vert_Number);

	Graph();
	Graph(const string& file_name);
	Graph(const Graph& rGraph);
	~Graph(void);

	void clear();

	Ptr<ControllerRouter> get_vertex(Ptr<ControllerRouter>  node_id);

	int get_edge_code(const Ptr<ControllerRouter> start_vertex_pt, const Ptr<ControllerRouter> end_vertex_pt) const;
	set<Ptr<ControllerRouter>>* get_vertex_set_pt(Ptr<ControllerRouter> vertex_, map<Ptr<ControllerRouter>, set<Ptr<ControllerRouter>>*>& vertex_container_index);

	double get_original_edge_weight(const Ptr<ControllerRouter> source, const Ptr<ControllerRouter> sink);

	double get_edge_weight(const Ptr<ControllerRouter> source, const Ptr<ControllerRouter> sink);
	void get_adjacent_vertices(Ptr<ControllerRouter> vertex, set<Ptr<ControllerRouter>>& vertex_set);
	void get_precedent_vertices(Ptr<ControllerRouter> vertex, set<Ptr<ControllerRouter>>& vertex_set);
	void add_incidency(Ptr<ControllerRouter> source, Ptr<ControllerRouter> sink, double edge_weight);
	void printEdgeNo();
	void printVertexNo();
	void printVertexInfo();
	void setVertexNo(int vertexNo);


	/// Methods for changing graph
	void remove_edge(const pair<int,int> edge)
	{
		m_stRemovedEdge.insert(edge);
	}

	void remove_vertex(const int vertex_id)
	{
		m_stRemovedVertexIds.insert(vertex_id);
	}

	void recover_removed_edges()
	{
		m_stRemovedEdge.clear();
	}

	void recover_removed_vertices()
	{
		m_stRemovedVertexIds.clear();
	}

	void recover_removed_edge(const pair<int,int> edge)
	{
		m_stRemovedEdge.erase(m_stRemovedEdge.find(edge));
	}

	void recover_removed_vertex(int vertex_id)
	{
		m_stRemovedVertexIds.erase(m_stRemovedVertexIds.find(vertex_id));
	}

private:
	void _import_from_file(const std::string& file_name);

};

}
}

#endif /* NDN_YANGRAPH_H */
