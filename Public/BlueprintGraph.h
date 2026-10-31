#pragma once
#include"GraphView.h"
#include"NodeGraphFactories.h"
#include <cstddef>

//contextual visual script graph, allows using functions and variables to do scripting
class BlueprintGraph : public NodeEditor::GraphView
{
public:
	struct ExecutionResult
	{
		bool Success = false;
		std::string Diagnostic;
		std::size_t StepsExecuted = 0;
	};
	static constexpr std::size_t DefaultStepLimit = 1024;
	BlueprintGraph();
	ExecutionResult Execute(std::size_t stepLimit = DefaultStepLimit);

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
	bool ExecuteFromHere();
	virtual bool Execute();
	virtual bool IsStartNode() const { return false; }
	virtual bool IsEndNode() const { return false; }
	virtual bool AllowsFlowInput() const { return true; }
	virtual bool AllowsFlowOutput() const { return true; }
};

class BlueprintFlowPin : public NodeEditor::GraphPin
{
public:
	BlueprintFlowPin();
};

class BlueprintStartNode : public BlueprintNode
{
public:
	BlueprintStartNode();
	bool Execute() override { return true; }
	bool IsStartNode() const override { return true; }
	bool AllowsFlowInput() const override { return false; }
};

class BlueprintEndNode : public BlueprintNode
{
public:
	BlueprintEndNode();
	bool Execute() override { return true; }
	bool IsEndNode() const override { return true; }
	bool AllowsFlowOutput() const override { return false; }
};

class BlueprintPrintMessageNode : public BlueprintNode
{
public:
	BlueprintPrintMessageNode();
	std::string Message = "Hello from my workflow";
	bool Execute() override;
protected:
	void DrawContents() override;
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

struct BlueprintStartNode_Factory :public GraphNodeFactory
{
	std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery* Registery)const override;
};

struct BlueprintEndNode_Factory :public GraphNodeFactory
{
	std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery*)const override;
};

struct BlueprintPrintMessageNode_Factory :public GraphNodeFactory
{
	std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery* Registery)const override;
};

struct BlueprintPin_Default_Factory :public GraphPinFactory
{
	std::shared_ptr<NodeEditor::GraphPin> Create(class NodeEditorRegistery* Registery)const override;
};

struct BlueprintPin_Flow_Factory :public GraphPinFactory
{
	std::shared_ptr<NodeEditor::GraphPin> Create(class NodeEditorRegistery* Registery)const override;
};

void RegisterBlueprintGraph();
