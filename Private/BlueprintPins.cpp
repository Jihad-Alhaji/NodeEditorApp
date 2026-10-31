#include "BlueprintPins.h"
#include "BlueprintGraph.h"
#include "BlueprintNodes.h"
#include "BlueprintNode.h"
#include "NodeEditorRegistery.h"
std::shared_ptr<NodeEditor::GraphPin> BlueprintPin_Default_Factory::Create(NodeEditorRegistery* Registery) const
{
	return std::make_shared<NodeEditor::GraphPin>();
}

std::shared_ptr<NodeEditor::GraphPin> BlueprintPin_Flow_Factory::Create(NodeEditorRegistery*) const
{
	return std::make_shared<BlueprintFlowPin>();
}



void RegisterBlueprintGraph()
{
	auto reg = NodeEditorRegistery::Get();
	reg->RegisterGraph("Blueprint", std::make_shared<BlueprintGraph_Factory>());

	reg->RegisterPin("default", std::make_shared<BlueprintPin_Default_Factory>());
	reg->RegisterPin("flow", std::make_shared<BlueprintPin_Flow_Factory>());
	reg->RegisterNode("Blueprint", "Start", std::make_shared<BlueprintStartNode_Factory>());
	reg->RegisterNode("Blueprint", "Print Message", std::make_shared<BlueprintPrintMessageNode_Factory>());
	{
		reg->RegisterNode("Blueprint", "End", std::make_shared<BlueprintEndNode_Factory>());
	}
	{
		auto d1 = std::make_shared<BlueprintNode_Factory>();
		d1->Title = "default node";
		d1->Inputs.push_back({ "default","in" });
		d1->Outputs.push_back({ "default","out" });
		reg->RegisterNode("Blueprint", "default", std::move(d1));
	}
	
	{
		auto d1 = std::make_shared<BlueprintNode_Factory>();
		d1->Title = "Execute";
		d1->Outputs.push_back({ "default","out" });
		reg->RegisterNode("Blueprint", "Execute", std::move(d1));
	}
}

