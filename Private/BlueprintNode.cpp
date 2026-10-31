#include "BlueprintNode.h"
#include "BlueprintGraph.h"
#include "NodeEditorRegistery.h"
#include <imgui/imgui.h>
#include <ArcLog/LogSystem.h>
BlueprintNode::BlueprintNode() :NodeEditor::GraphNode("")
{
}

void BlueprintNode::DrawContextMenu()
{
	GraphNode::DrawContextMenu();
	if (IsStartNode())
	{
		ImGui::Separator();
		if (ImGui::Selectable("Debug Execute From Here"))
		{
			if (!ExecuteFromHere())
			{
				Logging::LogSystem::ArcLogger.Send(Logging::LogVerbosity::ARCERROR,
					"Debug execution failed: Start must have exactly one connected flow output.");
			}
		}
	}
}

bool BlueprintNode::ExecuteFromHere()
{
	if (!IsStartNode()) return false;
	auto* graph = dynamic_cast<BlueprintGraph*>(GetGraph());
	return graph && graph->Execute().Success;
}

std::shared_ptr<NodeEditor::GraphNode> BlueprintNode_Factory::Create(NodeEditorRegistery* Registery) const
{
	auto n = std::make_shared<BlueprintNode>();

	n->Title = Title;
	for (const auto& input : Inputs)
	{
		if (input.Factory == "flow")
		{
			n->NodeSize.y += 40.f;
			break;
		}
	}
	n->SetSize(n->NodeSize);
	//spawn pins
	for (auto& i : Inputs) {
		auto p = Registery->SpawnPin(i.Factory);
		if (p)
		{
			p->SetName(i.Name);
			p->Type = NodeEditor::EPinType::Input;
			n->AddPin(std::move(p));
		}
	}

	for (auto& i : Outputs) {
		auto p = Registery->SpawnPin(i.Factory);
		if (p)
		{
			p->SetName(i.Name);
			p->Type = NodeEditor::EPinType::Output;
			n->AddPin(std::move(p));
		}
	}
	n->Function = Function;
	return n;
}




