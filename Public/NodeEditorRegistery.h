#pragma once
#include"NodeGraphFactories.h"
#include<string>
#include<vector>

//singleton class , database object of all graph , node, pin types
//alos act as a bridge class that fully encapsulates it implementation, this was done to make sure its api is consistent 
class NodeEditorRegistery
{
	NodeEditorRegistery();
	inline static NodeEditorRegistery* Singleton = nullptr;

	std::unique_ptr<class NEReg_Impl> Implementation;
public:
	static NodeEditorRegistery* Get();
	static NodeEditorRegistery& GetChecked();
	static void Initialize();

	void RegisterGraph(const std::string& Key, std::shared_ptr < GraphViewFactory>&& factory);
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
