#include "BlueprintNodes.h"
#include "NodeEditorRegistery.h"
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <ArcLog/LogSystem.h>
#include "blueprintPins.h"
BlueprintStartNode::BlueprintStartNode() : BlueprintNode()
{
	Title = "Start";
	NodeSize = { 160, 80 };
	SetSize(NodeSize);
}

BlueprintPrintMessageNode::BlueprintPrintMessageNode() : BlueprintNode()
{
	Title = "Print Message";
	NodeSize = { 280, 108 };
	Size = NodeSize;
}

bool BlueprintPrintMessageNode::Execute()
{
	Logging::LogSystem::ArcLogger.Send(Logging::LogVerbosity::NOTE, Message);
	return true;
}

void BlueprintPrintMessageNode::DrawContents()
{
	ImGui::SetCursorScreenPos(AbsoluteRect.Min + ImVec2(16.f, 52.f));
	ImGui::SetNextItemWidth(AbsoluteRect.Size().x - 32.f);
	ImGui::InputText("##Message", &Message);
}

std::shared_ptr<NodeEditor::GraphNode> BlueprintStartNode_Factory::Create(NodeEditorRegistery* Registery) const
{
	auto node = std::make_shared<BlueprintStartNode>();
	node->TypeKey = "Start";
	auto flowOut = std::make_shared<BlueprintFlowPin>();
	flowOut->SetName("flow");
	flowOut->PinKey = "exec.out";
	flowOut->Type = NodeEditor::EPinType::Output;
	node->AddPin(std::move(flowOut));
	return node;
}

std::shared_ptr<NodeEditor::GraphNode> BlueprintPrintMessageNode_Factory::Create(NodeEditorRegistery* Registery) const
{
	auto node = std::make_shared<BlueprintPrintMessageNode>();
	node->TypeKey = "Print Message";
	for (const auto& direction : { NodeEditor::EPinType::Input, NodeEditor::EPinType::Output })
	{
		auto pin = std::make_shared<BlueprintFlowPin>();
		pin->SetName("flow");
		pin->PinKey = direction == NodeEditor::EPinType::Input ? "exec.in" : "exec.out";
		pin->Type = direction;
		node->AddPin(std::move(pin));
	}
	return node;
}


