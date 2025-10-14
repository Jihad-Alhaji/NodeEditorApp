#pragma once
#include<memory>

namespace NodeEditor
{
	class GraphView;
	class GraphNode;
	class GraphPin;
}

struct GraphNodeFactory
{
	virtual std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery* Registery)const = 0;
};

struct GraphPinFactory
{
	virtual std::shared_ptr<NodeEditor::GraphPin> Create(class NodeEditorRegistery* Registery)const = 0;
};

struct GraphViewFactory
{
	virtual std::shared_ptr<NodeEditor::GraphView> Create(class NodeEditorRegistery* Registery)const = 0;
};
