#ifndef NDN_YANGRAPH_H
#define NDN_YANGRAPH_H

#pragma once
#include <iostream>


namespace ns3 {
namespace ndn {

using namespace std;

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



//////////////////////////////////////////////////////////////////////////
// A class for the object deletion
//////////////////////////////////////////////////////////////////////////
template<class T>
class DeleteFunc
{
public:
	void operator()(const T* it) const
	{
		delete it;
	}
};

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
	std::vector<BaseVertex*> m_vtVertexList;

public:
	BasePath(const std::vector<BaseVertex*>& vertex_list, double weight)
		:m_dWeight(weight)
	{
		m_vtVertexList.assign(vertex_list.begin(), vertex_list.end());
		m_nLength = m_vtVertexList.size();
	}
	~BasePath(void){}

	double Weight() const { return m_dWeight; }
	void Weight(double val) { m_dWeight = val; }

	int length() { return m_nLength; }

	BaseVertex* GetVertex(int i)
	{
		return m_vtVertexList.at(i);
	}

	bool SubPath(std::vector<BaseVertex*>& sub_path, BaseVertex* ending_vertex_pt)
	{

		for (std::vector<BaseVertex*>::const_iterator pos = m_vtVertexList.begin();
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
		for(std::vector<BaseVertex*>::const_iterator pos=m_vtVertexList.begin(); pos!=m_vtVertexList.end();++pos)
		{
			(*pos)->PrintOut(out_stream);
			//out_stream << "->";
			std::cout << "->";
		}
		//out_stream << std::endl <<  "*********************************************" << std::endl;
		std::cout << std::endl <<  "*********************************************" << std::endl;
	}
};

class Path : public BasePath
{
public:

	Path(const std::vector<BaseVertex*>& vertex_list, double weight):BasePath(vertex_list,weight){}

	// display the content
	void PrintOut(std::ostream& out_stream) const
	{
		//out_stream << "Cost: " << m_dWeight << " Length: " << m_vtVertexList.size() << std::endl;
		std::cout << "Cost: " << m_dWeight << " Length: " << m_vtVertexList.size() << std::endl;
		for(std::vector<BaseVertex*>::const_iterator pos=m_vtVertexList.begin(); pos!=m_vtVertexList.end();++pos)
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

	typedef set<BaseVertex*>::iterator VertexPtSetIterator;
	typedef map<BaseVertex*, set<BaseVertex*>*>::iterator BaseVertexPt2SetMapIterator;

protected: // members

	// Basic information
	map<BaseVertex*, set<BaseVertex*>*> m_mpFanoutVertices;
	map<BaseVertex*, set<BaseVertex*>*> m_mpFaninVertices;
	map<int, double> m_mpEdgeCodeWeight;
	vector<BaseVertex*> m_vtVertices;
	int m_nEdgeNum;
	int m_nVertexNum;

	map<int, BaseVertex*> m_mpVertexIndex;

	// Members for graph modification
	set<int> m_stRemovedVertexIds;
	set<pair<int,int> > m_stRemovedEdge;

public:

	// Constructors and Destructor
	Graph(const string& file_name);
	Graph(const Graph& rGraph);
	~Graph(void);

	void clear();

	BaseVertex* get_vertex(int node_id);

	int get_edge_code(const BaseVertex* start_vertex_pt, const BaseVertex* end_vertex_pt) const;
	set<BaseVertex*>* get_vertex_set_pt(BaseVertex* vertex_, map<BaseVertex*, set<BaseVertex*>*>& vertex_container_index);

	double get_original_edge_weight(const BaseVertex* source, const BaseVertex* sink);

	double get_edge_weight(const BaseVertex* source, const BaseVertex* sink);
	void get_adjacent_vertices(BaseVertex* vertex, set<BaseVertex*>& vertex_set);
	void get_precedent_vertices(BaseVertex* vertex, set<BaseVertex*>& vertex_set);

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
