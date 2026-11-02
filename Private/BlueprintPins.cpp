#include "BlueprintPins.h"
#include "BlueprintNodes.h"
#include "NodeEditorRegistery.h"

BlueprintFlowPin::BlueprintFlowPin()
{
	Role = NodeEditor::EPinRole::Flow;
}


std::shared_ptr<NodeEditor::GraphPin> BlueprintPin_Default_Factory::Create(NodeEditorRegistery* Registery) const
{
	return std::make_shared<NodeEditor::GraphPin>();
}

std::shared_ptr<NodeEditor::GraphPin> BlueprintPin_Flow_Factory::Create(NodeEditorRegistery*) const
{
	return std::make_shared<BlueprintFlowPin>();
}

