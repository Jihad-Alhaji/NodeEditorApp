#pragma once
#include<vector>
#include<set>
#include<map>
#include<ResourceManagement/ISerializationInterface.h>

enum GraphConstants
{
	InValidGraphIndex = -1
};

class GraphNode : public ResourceManagement::Serialization::ISerializationInterface
{
public:
	GraphNode();
	GraphNode(size_t id);
	virtual ~GraphNode() = default;

	inline size_t GetId()const { return Id; }
	inline void SetId(size_t newId) { Id = newId; }

	//return true if id is not invalidgraphindex
	inline bool IsValid()const { return Id != InValidGraphIndex; }

	bool Load(const Serialzer_t& Archive) override;
	void Save(Serialzer_t& Archive) override;
protected:
	size_t Id;
};

bool operator==(const GraphNode& a, const GraphNode& b);
bool operator==(const GraphNode& a, size_t id);

class GraphEdge :public ResourceManagement::Serialization::ISerializationInterface
{
public:
	GraphEdge();
	GraphEdge(size_t from, size_t to, double cost);
	GraphEdge(double cost);
	virtual ~GraphEdge() = default;

	inline size_t GetFrom()const { return From; };
	inline size_t GetTo()const { return To; };

	inline void SetFrom(size_t id) { From = id; };
	inline void SetTo(size_t id) { To = id; };

	inline double GetCost()const { return Cost; }
	inline void SetCost(double newCost) { Cost = newCost; }

	//resets the edge in one call
	void Reset(size_t from = InValidGraphIndex, size_t to = InValidGraphIndex, double cost = 0.0);

	//returns true if both from and to are valid indecies
	inline bool IsValid()const { return From != InValidGraphIndex && To != InValidGraphIndex; }

	bool operator<(const GraphEdge& other)const 
	{
		return To < other.To;
	}

	bool Load(const Serialzer_t& Archive) override;
	void Save(Serialzer_t& Archive) override;
protected:
	size_t From;
	size_t To;
	double Cost;
};
bool operator==(const GraphEdge& a, const GraphEdge& b);


template<class Node_t, class Edge_t>
class SparseGraph :public ResourceManagement::Serialization::ISerializationInterface
{
public:

	using EdgeList_t = typename std::set<Edge_t>;
	using NodeIterator = typename std::map<size_t, Node_t>::iterator;
	using ConstNodeIterator = typename std::map<size_t, Node_t>::const_iterator;
	using EdgeSetIterator = typename EdgeList_t::iterator;
	using ConstEdgeSetIterator = typename EdgeList_t::const_iterator;

	SparseGraph(bool bidirectional = false);

	inline const Node_t* GetNode(size_t id)const;
	inline Node_t* GetNode(size_t id);
	inline bool HasNode(size_t id)const;

	inline const Edge_t* GetEdge(size_t from, size_t to)const;
	inline Edge_t* GetEdge(size_t from, size_t to);
	inline bool HasEdge(size_t from, size_t to)const;

	void AddNode(Node_t&& newNode);
	void RemoveNode(size_t id);

	void AddEdge(Edge_t&& newEdge);
	void RemoveEdge(size_t from, size_t to);


	void Clear();
	inline bool IsEmpty()const;
	inline size_t GetNodesCount()const;

	bool Load(const Serialzer_t& Archive) override;
	void Save(Serialzer_t& Archive) override;

	
	

	// Custom iterator for all edges
	class EdgeIterator {
	public:
		using OuterMapIterator = typename std::map<size_t, EdgeList_t>::iterator;
		using InnerSetIterator = typename EdgeList_t::iterator;

		EdgeIterator(OuterMapIterator outerIt, OuterMapIterator outerEnd)
			: outerIt(outerIt), outerEnd(outerEnd) {
			if (outerIt != outerEnd) {
				innerIt = outerIt->second.begin();
				advanceToValid();
			}
			else 
			{
				// Set innerIt to a default state when at the end
				innerIt = InnerSetIterator();
			}
		}

		EdgeIterator& operator++() {
			if (outerIt == outerEnd) return *this;
			++innerIt;
			advanceToValid();
			return *this;
		}

		bool operator==(const EdgeIterator& other) const {
			if (outerIt == outerEnd && other.outerIt == other.outerEnd) {
				// Both are at the end
				return true;
			}
			return outerIt == other.outerIt && innerIt == other.innerIt;
		}

		bool operator!=(const EdgeIterator& other) const {
			return !(*this == other);
		}

		const Edge_t& operator*() const { return *innerIt; }

	private:
		void advanceToValid() {
			while (outerIt != outerEnd && innerIt == outerIt->second.end()) {
				++outerIt;
				if (outerIt != outerEnd) {
					innerIt = outerIt->second.begin();
				}
				else {
					// Ensure innerIt is set to a default state when at the end
					innerIt = InnerSetIterator();
				}
			}
		}

		OuterMapIterator outerIt;
		OuterMapIterator outerEnd;
		InnerSetIterator innerIt;
	};

	// Iterator methods for nodes
	NodeIterator beginNodes() { return Nodes.begin(); }
	NodeIterator endNodes() { return Nodes.end(); }
	ConstNodeIterator beginNodes() const { return Nodes.begin(); }
	ConstNodeIterator endNodes() const { return Nodes.end(); }

	// Iterator methods for all edges
	EdgeIterator beginEdges() { return EdgeIterator(Edges.begin(), Edges.end()); }
	EdgeIterator endEdges() { return EdgeIterator(Edges.end(), Edges.end()); }

	// Iterator methods for all reverse edges
	decltype(auto) rbeginEdges() { return EdgeIterator(rEdges.begin(), rEdges.end()); }
	decltype(auto) rendEdges() { return EdgeIterator(rEdges.end(), rEdges.end()); }

	// Iterator methods for a single edge set
	EdgeSetIterator beginEdgeSet(size_t nodeId) {
		return Edges[nodeId].begin();
	}
	EdgeSetIterator endEdgeSet(size_t nodeId) {
		return Edges[nodeId].end();
	}
	ConstEdgeSetIterator beginEdgeSet(size_t nodeId) const {
		auto it = Edges.find(nodeId);
		return (it != Edges.end()) ? it->second.begin() : typename EdgeList_t::const_iterator();
	}
	ConstEdgeSetIterator endEdgeSet(size_t nodeId) const {
		auto it = Edges.find(nodeId);
		return (it != Edges.end()) ? it->second.end() : typename EdgeList_t::const_iterator();
	}

	// Iterator methods for a single reverse edge set
	decltype(auto) rbeginEdgeSet(size_t nodeId) {
		return rEdges[nodeId].begin();
	}
	decltype(auto) rendEdgeSet(size_t nodeId) {
		return rEdges[nodeId].end();
	}
	decltype(auto) rbeginEdgeSet(size_t nodeId) const {
		auto it = rEdges.find(nodeId);
		return (it != rEdges.end()) ? it->second.begin() : typename EdgeList_t::const_iterator();
	}
	decltype(auto) rendEdgeSet(size_t nodeId) const {
		auto it = rEdges.find(nodeId);
		return (it != rEdges.end()) ? it->second.end() : typename EdgeList_t::const_iterator();
	}

	// Enable range-based for loop for nodes
	NodeIterator begin() { return beginNodes(); }
	NodeIterator end() { return endNodes(); }
	ConstNodeIterator begin() const { return beginNodes(); }
	ConstNodeIterator end() const { return endNodes(); }
protected:

	//bidirectinal graph
	bool bDigraph;
	std::map<size_t, EdgeList_t> Edges;
	std::map<size_t, std::set<size_t>> rEdges;
	std::map<size_t, Node_t> Nodes;
};

template<class Node_t, class Edge_t>
SparseGraph<Node_t, Edge_t>::SparseGraph(bool bidirectional) :bDigraph{ bidirectional }, Edges{}, Nodes{}
{
}

template<class Node_t, class Edge_t>
const Node_t* SparseGraph<Node_t, Edge_t>::GetNode(size_t id) const
{
	auto res = Nodes.find(id);
	if (res != Nodes.end())
	{
		return &res->second;
	}
	return nullptr;
}

template<class Node_t, class Edge_t>
Node_t* SparseGraph<Node_t, Edge_t>::GetNode(size_t id)
{
	auto res = Nodes.find(id);
	if (res != Nodes.end())
	{
		return &res->second;
	}
	return nullptr;
}

template<class Node_t, class Edge_t>
bool SparseGraph<Node_t, Edge_t>::HasNode(size_t id) const
{
	return Nodes.contains(id);
}

template<class Node_t, class Edge_t>
const Edge_t* SparseGraph<Node_t, Edge_t>::GetEdge(size_t from, size_t to) const
{
	if (auto eds = Edges.find(from); eds != Edges.end())
	{
		auto res = eds->second.find(Edge_t(from, to, 0.0));
		if (res != eds->second.end())
		{
			return res.operator->();
		}
	}
	return nullptr;
}

template<class Node_t, class Edge_t>
Edge_t* SparseGraph<Node_t, Edge_t>::GetEdge(size_t from, size_t to)
{
	if (auto eds = Edges.find(from); eds != Edges.end())
	{
		auto res = eds->second.find(Edge_t(from, to, 0.0));
		if (res != eds->second.end())
		{
			return  const_cast<Edge_t*>(res.operator->());
		}
	}
	return nullptr;
}

template<class Node_t, class Edge_t>
bool SparseGraph<Node_t, Edge_t>::HasEdge(size_t from, size_t to) const
{
	return GetEdge(from,to) != nullptr;
}

template<class Node_t, class Edge_t>
void SparseGraph<Node_t, Edge_t>::AddNode(Node_t&& newNode)
{
	size_t id = newNode.GetId();
	Edges.insert({  id,{} });
	rEdges.insert({ id,{} });
	Nodes.insert({ id ,std::move(newNode)});
}

template<class Node_t, class Edge_t>
void SparseGraph<Node_t, Edge_t>::RemoveNode(size_t id)
{
	Nodes.erase(id);
	
	auto es = Edges[id];
	Edges.erase(id);
	for (auto& e : es) 
	{
		RemoveEdge(e.GetFrom(), e.GetTo());
	}

	auto es = rEdges[id];
	rEdges.erase(id);
	for (auto& e : es)
	{
		RemoveEdge(e, id);
	}
}

template<class Node_t, class Edge_t>
void SparseGraph<Node_t, Edge_t>::AddEdge(Edge_t&& newEdge)
{
	size_t f = newEdge.GetFrom();
	if (auto eds = Edges.find(newEdge.GetFrom()); eds != Edges.end())
	{
		eds->second.insert(std::move(newEdge));
	}
	if (auto eds = rEdges.find(newEdge.GetTo()); eds != rEdges.end())
	{
		eds->second.insert(f);
	}
}

template<class Node_t, class Edge_t>
void SparseGraph<Node_t, Edge_t>::RemoveEdge(size_t from, size_t to)
{
	if (auto eds = Edges.find(from); eds != Edges.end())
	{
		eds->second.erase(Edge_t(from, to, 0.0));
	}

	if (auto eds = rEdges.find(to); eds != rEdges.end())
	{
		eds->second.erase(from);
	}
}

template<class Node_t, class Edge_t>
void SparseGraph<Node_t, Edge_t>::Clear()
{
	Edges.clear();
	rEdges.clear();
	Nodes.clear();
}

template<class Node_t, class Edge_t>
bool SparseGraph<Node_t, Edge_t>::IsEmpty() const
{
	return Nodes.empty();
}

template<class Node_t, class Edge_t>
size_t SparseGraph<Node_t, Edge_t>::GetNodesCount() const
{
	return Nodes.size();
}

template<class Node_t, class Edge_t>
bool SparseGraph<Node_t, Edge_t>::Load(const Serialzer_t& Archive)
{
	for (auto& n : Archive["Nodes"]) 
	{
		Node_t newNode{};
		newNode.Load(n);
		AddNode(std::move(newNode));
	}

	for (auto& e : Archive["Edges"])
	{
		Edge_t newEdge{};
		newEdge.Load(e);
		AddEdge(std::move(newEdge));
	}

	LoadProp(Archive, bDigraph);
	return true;
}

template<class Node_t, class Edge_t>
void SparseGraph<Node_t, Edge_t>::Save(Serialzer_t& Archive)
{
	Archive["Edges"] = Serialzer_t::array();
	Archive["Nodes"] = Serialzer_t::array();

	size_t counter = 0;
	for (auto& [_,n] : Nodes)
	{
		n.Save(Archive["Nodes"][counter++]);
	}
	counter = 0;
	for (auto& [_, eList] : Edges)
	{
		for (auto& e : eList) {
			const_cast<GraphEdge&>(e).Save(Archive["Edges"][counter++]);
		}
	}
	counter = 0;

	SaveProp(Archive, bDigraph);
}


