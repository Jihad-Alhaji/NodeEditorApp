#pragma once
#include "GraphNode.h"
#include "NodeGraphFactories.h"
#include <functional>
#include <string>
#include <utility>
#include <vector>
class BlueprintNode : public NodeEditor::GraphNode
{
public:
	struct ExecutionResult
	{
		enum class Outcome { Failure, Continue, Complete };
		Outcome Result = Outcome::Failure;
		std::string OutputPinKey;
		std::string Diagnostic;
		static ExecutionResult Continue(std::string outputPinKey) { return { Outcome::Continue, std::move(outputPinKey), {} }; }
		static ExecutionResult Complete() { return { Outcome::Complete, {}, {} }; }
		static ExecutionResult Failure(std::string diagnostic) { return { Outcome::Failure, {}, std::move(diagnostic) }; }
	};
    using Args_t = std::vector<std::shared_ptr<NodeEditor::GraphPin>>;
    using Function_t = std::function<bool(const Args_t&, const Args_t&)>;
    BlueprintNode();
    Function_t Function;
    void DrawContextMenu() override;
    bool ExecuteFromHere();
    virtual bool Execute();
    // Legacy nodes execute normally and continue through their existing flow output.
    // Branching nodes override this to return the selected stable flow PinKey.
    virtual ExecutionResult ExecuteFlow();
    virtual bool IsStartNode() const { return false; }
    virtual bool IsEndNode() const { return false; }
};
struct PinDef { std::string Factory; std::string Name; std::string PinKey; };
struct BlueprintNode_Factory : public GraphNodeFactory
{
    std::string Title; std::vector<PinDef> Inputs; std::vector<PinDef> Outputs; BlueprintNode::Function_t Function;
    std::string TypeKey;
    std::string GetTypeKey() const override { return TypeKey; }
    std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery*) const override;
};

