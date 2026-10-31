#pragma once
#include "BlueprintNode.h"
#include "GraphPin.h"
#include <string>
class BlueprintFlowPin : public NodeEditor::GraphPin 
{ 
	public: BlueprintFlowPin(); 
};

class BlueprintStartNode : public BlueprintNode 
{ 
	public: BlueprintStartNode(); 
	bool Execute() override { return true; } 
	bool IsStartNode() const override { return true; } 
	bool AllowsFlowInput() const override { return false; } 
};

class BlueprintEndNode : public BlueprintNode 
{ 
	public: BlueprintEndNode(); 
	bool Execute() override { return true; } 
	bool IsEndNode() const override { return true; } 
	bool AllowsFlowOutput() const override { return false; } 
};

class BlueprintPrintMessageNode : public BlueprintNode
{
	public: BlueprintPrintMessageNode(); 
	std::string Message = "Hello from my workflow"; 
	bool Execute() override;
	protected: void DrawContents() override;
};

struct BlueprintStartNode_Factory : public GraphNodeFactory 
{ 
	std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery*) const override; 
};

struct BlueprintEndNode_Factory : public GraphNodeFactory 
{ 
	std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery*) const override; 
};

struct BlueprintPrintMessageNode_Factory : public GraphNodeFactory 
{ 
	std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery*) const override; 
};


