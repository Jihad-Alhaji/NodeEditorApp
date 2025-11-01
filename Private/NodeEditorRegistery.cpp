#include "NodeEditorRegistery.h"
#include<cassert>
#include<unordered_map>


NodeEditorRegistery::NodeEditorRegistery()
{
	//add any initializatinos here
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

NodeEditorRegistery::GraphData* NodeEditorRegistery::GetGraphData(const std::string& key)
{
	auto iter = Graphs.find(key);
	if (iter == Graphs.end())
	{
		return nullptr;
	}
	return &iter->second;
}


void NodeEditorRegistery::RegisterGraph(const std::string& Key, std::shared_ptr < GraphViewFactory>&& factory)
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
std::vector<std::string> NodeEditorRegistery::GetRegisteredGraphs()
{
	std::vector<std::string> res;
	res.reserve(Graphs.size());
	for (const auto& i : Graphs) {
		res.push_back(i.first);
	}
	return res;
}
GraphViewFactory* NodeEditorRegistery::GetGraphFactory(const std::string& Key)
{
	GraphData* g = GetGraphData(Key);
	if (g == nullptr)
		return nullptr;

	return g->Factory.get();
}
std::shared_ptr<NodeEditor::GraphView> NodeEditorRegistery::SpawnGraph(const std::string& Key)
{
	auto gF = GetGraphFactory(Key);
	if (!gF)
	{
		return nullptr;
	}
	return SpawnGraph(*gF);
}
std::shared_ptr<NodeEditor::GraphView> NodeEditorRegistery::SpawnGraph(const GraphViewFactory& factory)
{
	return factory.Create(NodeEditorRegistery::Get());
}
void NodeEditorRegistery::UnregisterGraph(const std::string& Key)
{
	Graphs.erase(Key);
}

void NodeEditorRegistery::RegisterNode(const std::string& GraphKey, const std::string& Key, std::shared_ptr < GraphNodeFactory>&& factory)
{
	auto g = GetGraphData(GraphKey);
	if (g == nullptr)
		return;

	g->Nodes[Key] = std::move(factory);
}
std::vector<std::string> NodeEditorRegistery::GetRegisteredNodes(const std::string& GraphKey)
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
GraphNodeFactory* NodeEditorRegistery::GetNodeFactory(const std::string& GraphKey, const std::string& Key)
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
std::shared_ptr<NodeEditor::GraphNode> NodeEditorRegistery::SpawnNode(const std::string& GraphKey, const std::string& Key)
{
	auto nF = GetNodeFactory(GraphKey, Key);
	if (!nF)
	{
		return nullptr;
	}
	return SpawnNode(*nF);
}
std::shared_ptr<NodeEditor::GraphNode> NodeEditorRegistery::SpawnNode(const GraphNodeFactory& factory)
{
	return factory.Create(NodeEditorRegistery::Get());
}
void NodeEditorRegistery::UnregisterNode(const std::string& GraphKey, const std::string& Key)
{
	auto g = GetGraphData(GraphKey);
	if (g)
	{
		g->Nodes.erase(Key);
	}
}

std::vector<std::string> NodeEditorRegistery::GetRegisteredPins()
{
	std::vector<std::string> res;
	res.reserve(Pins.size());
	for (const auto& i : Pins) {
		res.push_back(i.first);
	}
	return res;
}
void NodeEditorRegistery::RegisterPin(const std::string& Key, std::shared_ptr<GraphPinFactory>&& factory)
{
	Pins[Key] = std::move(factory);
}
GraphPinFactory* NodeEditorRegistery::GetPinFactory(const std::string& Key)
{
	auto iter = Pins.find(Key);
	if (iter == Pins.end())
	{
		return nullptr;
	}
	return iter->second.get();
}
std::shared_ptr<NodeEditor::GraphPin> NodeEditorRegistery::SpawnPin(const std::string& Key)
{
	auto pF = GetPinFactory(Key);
	if (pF == nullptr)
	{
		return nullptr;
	}
	return SpawnPin(*pF);
}
std::shared_ptr<NodeEditor::GraphPin> NodeEditorRegistery::SpawnPin(const GraphPinFactory& factory)
{
	return factory.Create(NodeEditorRegistery::Get());
}
void NodeEditorRegistery::UnregisterPin(const std::string& Key)
{
	Pins.erase(Key);
}

void NodeEditorRegistery::UnregisterAll()
{
	Graphs.clear();
	Pins.clear();
}

