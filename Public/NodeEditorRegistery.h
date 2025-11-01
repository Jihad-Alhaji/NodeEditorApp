#pragma once
#include"NodeGraphFactories.h"
#include<string>
#include<vector>
#include<unordered_map>


//singleton class , database object of all graph , node, pin types
class NodeEditorRegistery
{
	struct GraphData
	{
		std::shared_ptr<GraphViewFactory> Factory;
		std::unordered_map < std::string, std::shared_ptr<GraphNodeFactory>> Nodes;
	};

	NodeEditorRegistery();
	inline static NodeEditorRegistery* Singleton = nullptr;

	std::unordered_map<std::string, GraphData> Graphs;
	std::unordered_map<std::string, std::shared_ptr<GraphPinFactory	>> Pins;

	GraphData* GetGraphData(const std::string& key);
	void UnregisterAll();
public:
	static NodeEditorRegistery* Get();
	static NodeEditorRegistery& GetChecked();
	static void Initialize();

	void RegisterGraph(const std::string& Key, std::shared_ptr <GraphViewFactory>&& factory);
	std::vector<std::string> GetRegisteredGraphs();
	GraphViewFactory* GetGraphFactory(const std::string& Key);
	std::shared_ptr<NodeEditor::GraphView> SpawnGraph(const std::string& Key);
	std::shared_ptr<NodeEditor::GraphView> SpawnGraph(const GraphViewFactory& factory);
	void UnregisterGraph(const std::string& Key);

	void RegisterNode(const std::string& GraphKey, const std::string& Key, std::shared_ptr < GraphNodeFactory>&& factory);
	std::vector<std::string> GetRegisteredNodes(const std::string& GraphKey);
	GraphNodeFactory* GetNodeFactory(const std::string& GraphKey, const std::string& Key);
	std::shared_ptr<NodeEditor::GraphNode> SpawnNode(const std::string& GraphKey,const std::string& Key);
	std::shared_ptr<NodeEditor::GraphNode> SpawnNode(const GraphNodeFactory& factory);
	void UnregisterNode(const std::string& GraphKey,const std::string& Key);

	std::vector<std::string> GetRegisteredPins();
	void RegisterPin(const std::string& Key, std::shared_ptr < GraphPinFactory>&& factory);
	GraphPinFactory* GetPinFactory(const std::string& Key);
	std::shared_ptr<NodeEditor::GraphPin> SpawnPin(const std::string& Key);
	std::shared_ptr<NodeEditor::GraphPin> SpawnPin(const GraphPinFactory& factory);
	void UnregisterPin(const std::string& Key);
};
