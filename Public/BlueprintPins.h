#pragma once
#include "GraphPin.h"
#include "NodeGraphFactories.h"
#include <memory>

class BlueprintFlowPin : public NodeEditor::GraphPin
{
	public: BlueprintFlowPin();
};

class BlueprintPin_Default_Factory : public GraphPinFactory 
{ 
	public: std::shared_ptr<NodeEditor::GraphPin> Create(class NodeEditorRegistery*) const override; 
};
class BlueprintPin_Flow_Factory : public GraphPinFactory 
{ 
	public: std::shared_ptr<NodeEditor::GraphPin> Create(class NodeEditorRegistery*) const override; 
};



