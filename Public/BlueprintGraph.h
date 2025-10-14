#pragma once
#include"GraphView.h"
#include"NodeGraphFactories.h"

class BlueprintGraph : public NodeEditor::GraphView
{
public:
	BlueprintGraph();
protected:
	//void DrawConnection(const NodeEditor::GraphConnection& con)override;
	void DrawContextMenu()override;

	std::vector<std::string> ctx_Nodes;
};

struct BlueprintGraph_Factory :public GraphViewFactory
{
	std::shared_ptr<NodeEditor::GraphView> Create(class NodeEditorRegistery* Registery)const override;
};

struct PinDef
{
	std::string Factory;
	std::string Name;
};

struct BlueprintNode_Factory :public GraphNodeFactory
{
	std::string Title;
	std::vector<PinDef> Inputs;
	std::vector<PinDef> Outputs;

	std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery* Registery)const override;
};

struct BlueprintPin_Default_Factory :public GraphPinFactory
{
	std::shared_ptr<NodeEditor::GraphPin> Create(class NodeEditorRegistery* Registery)const override;
};
void RegisterBlueprintGraph();