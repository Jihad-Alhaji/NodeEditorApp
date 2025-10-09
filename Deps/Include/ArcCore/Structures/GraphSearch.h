#pragma once
#include"AdjacencyMat.h"
#include"SparseGraph.h"
#include<vector>
#include<queue>
#include<stack>

class GraphSearch {
public:
	using Edge_t = typename std::pair<size_t, size_t>;
	struct NodeSearchData
	{
		bool Visited = false;
		size_t Parent = InValidGraphIndex;
	};

	//does a depth first search through the graph and returns the first rout to the target if found
	static std::vector<size_t> DepthFirstSearch(const AdjacencyMat& graph, size_t start, size_t target);
	static std::vector<size_t> BreadthFirstSearch(const AdjacencyMat& graph, size_t start, size_t target);
	
	template<class NodeType, class EdgeType>
	static std::vector<size_t> DepthFirstSearch(const SparseGraph<NodeType, EdgeType>& graph, size_t start, size_t target);
	
	template<class NodeType, class EdgeType>
	static std::vector<size_t> BreadthFirstSearch(const SparseGraph<NodeType, EdgeType>& graph, size_t start, size_t target);
	
	template<class NodeType, class EdgeType>
	static std::vector<size_t> AStarSearch(const SparseGraph<NodeType, EdgeType>& graph, size_t start, size_t target, std::function<double(size_t, size_t)> heuristic);
};

template<class NodeType, class EdgeType>
inline std::vector<size_t> GraphSearch::DepthFirstSearch(const SparseGraph<NodeType, EdgeType>& graph, size_t start, size_t target)
{
	// Visited array to track visited nodes
	std::map<size_t, NodeSearchData> Data{};
	for (auto& [i, n] : graph) {
		Data[i] = NodeSearchData();
	}

	// Stack to manage DFS traversal
	std::stack<Edge_t> Stack;

	Stack.push({ start,start });

	size_t Counter = 0;
	// DFS Iterative traversal
	while (!Stack.empty())
	{
		Edge_t current = Stack.top();
		Stack.pop();


		// Skip if already visited
		if (Data[current.second].Visited) {
			continue;
		}

		//store route to parent
		Data[current.second].Parent = current.first;

		// Mark the current node as visited and call the visit callback
		Data[current.second].Visited = true;
		Counter++;
		if (current.second == target)
		{
			break;
		}

		// Iterate over all outgoing edges from the current node
		for (auto i = graph.beginEdgeSet(current.second); i != graph.endEdgeSet(current.second); i++)
		{
			size_t neighbor = i->GetTo();
			if (!Data[neighbor].Visited) {
				Stack.push({ current.second, neighbor });
			}
		}
	}

	if (Data[target].Parent == InValidGraphIndex)
	{
		//no path found
		return {};
	}

	std::vector<size_t> Results;
	size_t nd = target;
	Results.push_back(nd);
	while (nd != start)
	{
		nd = Data[nd].Parent;
		Results.push_back(nd);
	}
	std::reverse(Results.begin(), Results.end());
	return Results;
}


template<class NodeType, class EdgeType>
inline std::vector<size_t> GraphSearch::BreadthFirstSearch(const SparseGraph<NodeType, EdgeType>& graph, size_t start, size_t target)
{
	// Visited array to track visited nodes
	std::map<size_t, NodeSearchData> Data{};
	for (auto& [i, n] : graph) {
		Data[i] = NodeSearchData();
	}

	// Stack to manage DFS traversal
	std::queue<Edge_t> Queue;

	Queue.push({ start,start });

	size_t Counter = 0;
	// DFS Iterative traversal
	while (!Queue.empty())
	{
		Edge_t current = Queue.front();
		Queue.pop();


		// Skip if already visited
		if (Data[current.second].Visited) {
			continue;
		}

		//store route to parent
		Data[current.second].Parent = current.first;

		// Mark the current node as visited and call the visit callback
		Data[current.second].Visited = true;
		Counter++;
		if (current.second == target)
		{
			break;
		}

		// Iterate over all outgoing edges from the current node
		for (auto i = graph.beginEdgeSet(current.second); i != graph.endEdgeSet(current.second); i++)
		{
			size_t neighbor = i->GetTo();
			if (!Data[neighbor].Visited) {
				Queue.push({ current.second, neighbor });
			}
		}
	}

	if (Data[target].Parent == InValidGraphIndex)
	{
		//no path found
		return {};
	}

	std::vector<size_t> Results;
	size_t nd = target;
	Results.push_back(nd);
	while (nd != start)
	{
		nd = Data[nd].Parent;
		Results.push_back(nd);
	}
	std::reverse(Results.begin(), Results.end());
	return Results;
}

template<class NodeType, class EdgeType>
inline std::vector<size_t> GraphSearch::AStarSearch(
	const SparseGraph<NodeType, EdgeType>& graph,
	size_t startId,
	size_t goalId,
	std::function<double(size_t, size_t)> heuristic)
{
	// Priority queue for open set (min-heap based on f-score = g_scaore + heurustic)
	using PQElement = std::pair<double, size_t>; // (f-score, nodeId)
	std::priority_queue<PQElement, std::vector<PQElement>, std::greater<>> openSet;

	// Maps for storing g-scores and f-scores
	//best path tree
	std::unordered_map<size_t, double> gScore;

	// Map for reconstructing the path
	std::unordered_map<size_t, size_t> cameFrom;

	// Initialize scores
	gScore[startId] = 0.0;
	
	// Add start node to open set
	openSet.emplace(heuristic(startId, goalId), startId);

	while (!openSet.empty()) {
		// Get the node with the lowest f-score
		size_t current = openSet.top().second;
		openSet.pop();

		// If goal is reached, reconstruct and return the path
		if (current == goalId) {
			std::vector<size_t> path;
			while (current != startId) {
				path.push_back(current);
				current = cameFrom[current];
			}
			path.push_back(startId);
			std::reverse(path.begin(), path.end());
			return path;
		}

		// Process neighbors
		for (auto it = graph.beginEdgeSet(current); it != graph.endEdgeSet(current); ++it) {
			const auto& edge = *it;
			size_t neighbor = edge.GetTo();
			double tentativeGScore = gScore[current] + edge.GetCost();

			//do edge relaxation
			if (!gScore.contains(neighbor) || tentativeGScore < gScore[neighbor]) {
				// Update scores and path
				cameFrom[neighbor] = current;
				gScore[neighbor] = tentativeGScore;
	
				// Add neighbor to the open set
				openSet.emplace(tentativeGScore + heuristic(neighbor, goalId), neighbor);
			}
		}
	}

	// If we reach here, no path was found
	return {};
}

template<class GraphNode, class GraphEdge>
std::vector<size_t> TopologicalSort(SparseGraph<GraphNode, GraphEdge>& graph) {
	std::map<size_t, size_t> inDegree;
	std::vector<size_t> result;
	std::queue<size_t> zeroInDegree;

	// Initialize in-degree of all nodes to 0
	for (const auto& [id, node] : graph)
	{
		inDegree[id] = 0;
	}

	// Compute in-degree by checking all edges
	for (auto i = graph.beginEdges(); i != graph.endEdges(); ++i)
	{
		auto& e = (*i);
		inDegree[e.GetFrom()]++;
	}

	// Find all nodes with 0 in-degree
	for (const auto& [id, deg] : inDegree)
	{
		if (deg == 0)
		{
			zeroInDegree.push(id);
		}
	}

	// Kahn’s algorithm
	while (!zeroInDegree.empty())
	{
		size_t nodeId = zeroInDegree.front();
		zeroInDegree.pop();
		result.push_back(nodeId);

		for (auto i = graph.rbeginEdgeSet(nodeId); i != graph.rendEdgeSet(nodeId); i++)
		{
			size_t f = (*i);
			if (--inDegree[f] == 0)
			{
				zeroInDegree.push(f);
			}
		}
	}
	// Check for cycles
	if (result.size() != graph.GetNodesCount()) {
		return {};
	}
	return result;
}
