#include "NodeEditorRegistery.h"
#include<cassert>
#include<unordered_map>
struct GraphData
{
	std::shared_ptr<GraphViewFactory> Factory;
	std::unordered_map < std::string, std::shared_ptr<GraphNodeFactory>> Nodes;
};
class NEReg_Impl
{
public:
	std::unordered_map<std::string, GraphData> Graphs;
	std::unordered_map<std::string, std::shared_ptr<GraphPinFactory	>> Pins;

	GraphData* GetGraphData(const std::string& key)
	{
		auto iter = Graphs.find(key);
		if (iter == Graphs.end())
		{
			return nullptr;
		}
		return &iter->second;
	}
	void RegisterGraph(const std::string& Key, std::shared_ptr<GraphViewFactory> && factory)
	{
		auto iter = Graphs.find(Key);
		if (iter == Graphs.end())
		{
			Graphs[Key] = GraphData{ std::move(factory) };
		}
		else
		{
			iter->second.Factory = std::move(factory);
		}
	}
	std::vector<std::string> GetRegisteredGraphs()
	{
		std::vector<std::string> res;
		res.reserve(Graphs.size());
		for (const auto& i : Graphs) {
			res.push_back(i.first);
		}
		return res;
	}
	GraphViewFactory* GetGraphFactory(const std::string& Key)
	{
		GraphData* g = GetGraphData(Key);
		if (g == nullptr)
			return nullptr;

		return g->Factory.get();
	}
	std::shared_ptr<NodeEditor::GraphView> SpawnGraph(const std::string& Key)
	{
		auto gF = GetGraphFactory(Key);
		if (!gF)
		{
			return nullptr;
		}
		return SpawnGraph(*gF);
	}
	std::shared_ptr<NodeEditor::GraphView> SpawnGraph(const GraphViewFactory& factory)
	{
		return factory.Create(NodeEditorRegistery::Get());
	}
	void UnregisterGraph(const std::string& Key)
	{
		Graphs.erase(Key);
	}
	
	void RegisterNode(const std::string& GraphKey, const std::string& Key, std::shared_ptr<GraphNodeFactory>&& factory)
	{
		auto g = GetGraphData(GraphKey);
		if (g == nullptr)
			return;

		g->Nodes[Key] = std::move(factory);
	}
	std::vector<std::string> GetRegisteredNodes(const std::string& GraphKey)
	{
		std::vector<std::string> res;
		auto g = GetGraphData(GraphKey);
		if (g == nullptr)
			return res;

		auto& nodes = g->Nodes;
		res.reserve(nodes.size());
		for (const auto& i : nodes) {
			res.push_back(i.first);
		}
		return res;
	}
	GraphNodeFactory* GetNodeFactory(const std::string& GraphKey, const std::string& Key)
	{
		std::vector<std::string> res;
		auto g = GetGraphData(GraphKey);
		if (g == nullptr)
			return nullptr;

		auto nIter = g->Nodes.find(Key);
		if (nIter == g->Nodes.end())
			return nullptr;

		return nIter->second.get();
	}
	std::shared_ptr<NodeEditor::GraphNode> SpawnNode(const std::string& GraphKey, const std::string& Key)
	{
		auto nF = GetNodeFactory(GraphKey, Key);
		if (!nF)
		{
			return nullptr;
		}
		return SpawnNode(*nF);
	}
	std::shared_ptr<NodeEditor::GraphNode> SpawnNode(const GraphNodeFactory& factory)
	{
		return factory.Create(NodeEditorRegistery::Get());
	}
	void UnregisterNode(const std::string& GraphKey, const std::string& Key)
	{
		auto g = GetGraphData(GraphKey);
		if (g)
		{
			g->Nodes.erase(Key);
		}
	}
	
	std::vector<std::string> GetRegisteredPins()
	{
		std::vector<std::string> res;
		res.reserve(Pins.size());
		for (const auto& i : Pins) {
			res.push_back(i.first);
		}
		return res;
	}
	void RegisterPin(const std::string& Key, std::shared_ptr<GraphPinFactory>&& factory)
	{
		Pins[Key] = std::move(factory);
	}
	GraphPinFactory* GetPinFactory(const std::string& Key)
	{
		auto iter = Pins.find(Key);
		if (iter == Pins.end())
		{
			return nullptr;
		}
		return iter->second.get();
	}
	std::shared_ptr<NodeEditor::GraphPin> SpawnPin(const std::string& Key)
	{
		auto pF = GetPinFactory(Key);
		if (pF == nullptr)
		{
			return nullptr;
		}
		return SpawnPin(*pF);
	}
	std::shared_ptr<NodeEditor::GraphPin> SpawnPin(const GraphPinFactory& factory)
	{
		return factory.Create(NodeEditorRegistery::Get());
	}
	void UnregisterPin(const std::string& Key)
	{
		Pins.erase(Key);
	}


	void UnregisterAll()
	{
		Graphs.clear();
		Pins.clear();
	}
};

NodeEditorRegistery::NodeEditorRegistery()
{
	//perform any loading here
	Implementation.reset(new NEReg_Impl());
}

NodeEditorRegistery* NodeEditorRegistery::Get()
{
	return Singleton;
}

NodeEditorRegistery& NodeEditorRegistery::GetChecked()
{
	assert(Singleton);
	return *Singleton;
}

void NodeEditorRegistery::Initialize()
{
	if (Singleton)
	{
		return;
	}
	Singleton = new NodeEditorRegistery();
}

void NodeEditorRegistery::RegisterGraph(const std::string& Key, std::shared_ptr < GraphViewFactory>&& factory)
{
	Implementation->RegisterGraph(Key, std::move(factory));
}
std::vector<std::string> NodeEditorRegistery::GetRegisteredGraphs()
{
	return Implementation->GetRegisteredGraphs();
}
GraphViewFactory* NodeEditorRegistery::GetGraphFactory(const std::string& Key) {
	return Implementation->GetGraphFactory(Key);
}
std::shared_ptr<NodeEditor::GraphView> NodeEditorRegistery::SpawnGraph(const std::string& Key)
{
	return Implementation->SpawnGraph(Key);
}
std::shared_ptr<NodeEditor::GraphView> NodeEditorRegistery::SpawnGraph(const GraphViewFactory& factory)
{
	return Implementation->SpawnGraph(factory);
}
void NodeEditorRegistery::UnregisterGraph(const std::string& Key)
{
	Implementation->UnregisterGraph(Key);
}

void NodeEditorRegistery::RegisterNode(const std::string& GraphKey, const std::string& Key, std::shared_ptr < GraphNodeFactory>&& factory)
{
	Implementation->RegisterNode(GraphKey, Key, std::move(factory));
}
std::vector<std::string> NodeEditorRegistery::GetRegisteredNodes(const std::string& GraphKey)
{
	return Implementation->GetRegisteredNodes(GraphKey);
}
GraphNodeFactory* NodeEditorRegistery::GetNodeFactory(const std::string& GraphKey, const std::string& Key)
{
	return Implementation->GetNodeFactory(GraphKey, Key);
}
std::shared_ptr<NodeEditor::GraphNode> NodeEditorRegistery::SpawnNode(const std::string& GraphKey, const std::string& Key)
{
	return Implementation->SpawnNode(GraphKey, Key);
}
std::shared_ptr<NodeEditor::GraphNode> NodeEditorRegistery::SpawnNode(const GraphNodeFactory& factory)
{
	return Implementation->SpawnNode(factory);
}
void NodeEditorRegistery::UnregisterNode(const std::string& GraphKey, const std::string& Key)
{
	Implementation->UnregisterNode(GraphKey, Key);
}

std::vector<std::string> NodeEditorRegistery::GetRegisteredPins()
{
	return Implementation->GetRegisteredPins();
}
void NodeEditorRegistery::RegisterPin(const std::string& Key, std::shared_ptr < GraphPinFactory>&& factory)
{
	Implementation->RegisterPin(Key, std::move(factory));
}
GraphPinFactory* NodeEditorRegistery::GetPinFactory(const std::string& Key)
{
	return Implementation->GetPinFactory(Key);
}
std::shared_ptr<NodeEditor::GraphPin> NodeEditorRegistery::SpawnPin(const std::string& Key)
{
	return Implementation->SpawnPin(Key);
}
std::shared_ptr<NodeEditor::GraphPin> NodeEditorRegistery::SpawnPin(const GraphPinFactory& factory)
{
	return Implementation->SpawnPin(factory);
}
void NodeEditorRegistery::UnregisterPin(const std::string& Key)
{
	Implementation->UnregisterPin(Key);
}

