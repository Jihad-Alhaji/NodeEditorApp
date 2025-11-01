#pragma once
#include"GraphView.h"
#include"NodeGraphFactories.h"

//contextual visual script graph, allows using functions and variables to do scripting
class BlueprintGraph : public NodeEditor::GraphView
{
public:
	BlueprintGraph();

protected:
	//void DrawConnection(const NodeEditor::GraphConnection& con)override;
	void DrawContextMenu()override;

	std::vector<std::string> ctx_Nodes;//node keys cached from
};

struct BlueprintGraph_Factory :public GraphViewFactory
{
	std::shared_ptr<NodeEditor::GraphView> Create(class NodeEditorRegistery* Registery)const override {
		return std::make_shared<BlueprintGraph>();
	}
};


//the basic node class used in a blueprint graph
class BlueprintNode : public NodeEditor::GraphNode
{
public:
	using Args_t = typename std::vector<std::shared_ptr<NodeEditor::GraphPin>>;
	using Function_t = typename std::function<bool(const Args_t&/*inputs*/, const Args_t&/*outputs*/)>;
	BlueprintNode();
	Function_t Function;

	void DrawContextMenu()override;
	virtual bool Execute();
};

//lightweight deffinitions of a pin, used for spawning a pin into a node
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
	BlueprintNode::Function_t Function;

	std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery* Registery)const override;
};

struct BlueprintPin_Default_Factory :public GraphPinFactory
{
	std::shared_ptr<NodeEditor::GraphPin> Create(class NodeEditorRegistery* Registery)const override;
};

void RegisterBlueprintGraph();