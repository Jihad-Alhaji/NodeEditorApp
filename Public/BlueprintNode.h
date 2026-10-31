#pragma once
#include "GraphNode.h"
#include "NodeGraphFactories.h"
#include <functional>
#include <string>
#include <vector>
class BlueprintNode : public NodeEditor::GraphNode
{
public:
    using Args_t = std::vector<std::shared_ptr<NodeEditor::GraphPin>>;
    using Function_t = std::function<bool(const Args_t&, const Args_t&)>;
    BlueprintNode();
    Function_t Function;
    void DrawContextMenu() override;
    bool ExecuteFromHere();
    virtual bool Execute();
    virtual bool IsStartNode() const { return false; }
    virtual bool IsEndNode() const { return false; }
    virtual bool AllowsFlowInput() const { return true; }
    virtual bool AllowsFlowOutput() const { return true; }
};
struct PinDef { std::string Factory; std::string Name; };
struct BlueprintNode_Factory : public GraphNodeFactory
{
    std::string Title; std::vector<PinDef> Inputs; std::vector<PinDef> Outputs; BlueprintNode::Function_t Function;
    std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery*) const override;
};

