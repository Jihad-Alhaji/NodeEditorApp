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
	n->TypeKey = TypeKey;

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
			p->PinKey = i.PinKey;
			p->Type = NodeEditor::EPinType::Input;
			n->AddPin(std::move(p));
		}
	}

	for (auto& i : Outputs) {
		auto p = Registery->SpawnPin(i.Factory);
		if (p)
		{
			p->SetName(i.Name);
			p->PinKey = i.PinKey;
			p->Type = NodeEditor::EPinType::Output;
			n->AddPin(std::move(p));
		}
	}
	n->Function = Function;
	return n;
}


bool BlueprintNode::Execute()
{
	if (!Function) return false;
	return Function(Inputs, Outputs);
}

BlueprintNode::ExecutionResult BlueprintNode::ExecuteFlow()
{
	if (!Execute()) return ExecutionResult::Failure("Node execution failed: " + Title + " (ID " + std::to_string(NodeId) + ").");
	std::shared_ptr<NodeEditor::GraphPin> flowOutput;
	for (const auto& pin : Outputs)
		if (pin && pin->Role == NodeEditor::EPinRole::Flow)
		{
			if (flowOutput) return ExecutionResult::Failure("Node " + Title + " must select a flow output because it defines multiple flow outputs.");
			flowOutput = pin;
		}
	if (flowOutput) return ExecutionResult::Continue(flowOutput->PinKey);
	return ExecutionResult::Failure("Node " + Title + " completed without a flow output pin.");
}

