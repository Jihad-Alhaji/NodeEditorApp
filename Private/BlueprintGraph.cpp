#include "BlueprintGraph.h"
#include"NodeEditorRegistery.h"
#include<imgui/imgui.h>
#include<ArcLog/LogSystem.h>
#include<imgui/imgui_stdlib.h>

BlueprintGraph::BlueprintGraph()
{
	Name = "Blueprint";
	ctx_Nodes = NodeEditorRegistery::GetChecked().GetRegisteredNodes("Blueprint");
}

void BlueprintGraph::DrawContextMenu()
{
	GraphView::DrawContextMenu();
	ImGui::Separator();
	if (ImGui::Selectable("Execute Graph"))
	{

	}
	ImGui::SeparatorText("Add Nodes");
	for (auto& c : ctx_Nodes) 
	{
		if (ImGui::Selectable(c.c_str()))
		{
			auto n = NodeEditorRegistery::Get()->SpawnNode("Blueprint", c);
			if (n)
			{
				n->GraphPos = ScreenToGraph(ImGui::GetMousePos());
				AddNode(std::move(n));
			}
			break;
		}
	}
}


BlueprintNode::BlueprintNode() :NodeEditor::GraphNode("")
{
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
	auto node = std::make_shared<BlueprintNode>();
	node->Title = "Start";
	node->NodeSize = { 160, 80 };
	node->SetSize(node->NodeSize);
	auto flowOut = Registery->SpawnPin("flow");
	flowOut->SetName("flow");
	flowOut->Type = NodeEditor::EPinType::Output;
	node->AddPin(std::move(flowOut));
	return node;
}

std::shared_ptr<NodeEditor::GraphNode> BlueprintPrintMessageNode_Factory::Create(NodeEditorRegistery* Registery) const
{
	auto node = std::make_shared<BlueprintPrintMessageNode>();
	for (const auto& direction : { NodeEditor::EPinType::Input, NodeEditor::EPinType::Output })
	{
		auto pin = Registery->SpawnPin("flow");
		pin->SetName("flow");
		pin->Type = direction;
		node->AddPin(std::move(pin));
	}
	return node;
}

void BlueprintNode::DrawContextMenu()
{
	GraphNode::DrawContextMenu();
	if (Title == "Start")
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
	if (Title != "Start" || Outputs.size() != 1 || !Outputs.front() ||
		Outputs.front()->Role != NodeEditor::EPinRole::Flow || Outputs.front()->Connections.size() != 1)
	{
		return false;
	}

	auto connection = Outputs.front()->Connections.begin()->second.lock();
	if (!connection)
	{
		return false;
	}
	auto source = connection->From.lock();
	auto destination = connection->To.lock();
	if (!source || !destination || source != Outputs.front() ||
		destination->Type != NodeEditor::EPinType::Input || destination->Role != NodeEditor::EPinRole::Flow ||
		source->ParentNode.lock().get() != this ||
		source->ParentNode.lock() == destination->ParentNode.lock())
	{
		return false;
	}
	auto destinationNode = destination->ParentNode.lock();
	if (!destinationNode)
	{
		return false;
	}
	auto printNode = std::dynamic_pointer_cast<BlueprintPrintMessageNode>(destinationNode);
	if (!printNode)
	{
		return false;
	}
	return printNode->Execute();
}

bool BlueprintNode::Execute()
{
	if (!Function)
	{
		return false;
	}
	return Function(Inputs, Outputs);
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


std::shared_ptr<NodeEditor::GraphPin> BlueprintPin_Default_Factory::Create(NodeEditorRegistery* Registery) const
{
	return std::make_shared<NodeEditor::GraphPin>();
}

std::shared_ptr<NodeEditor::GraphPin> BlueprintPin_Flow_Factory::Create(NodeEditorRegistery*) const
{
	auto pin = std::make_shared<NodeEditor::GraphPin>();
	pin->Role = NodeEditor::EPinRole::Flow;
	return pin;
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
		auto end = std::make_shared<BlueprintNode_Factory>();
		end->Title = "End";
		end->Function = [](const BlueprintNode::Args_t&, const BlueprintNode::Args_t&) { return true; };
		end->Inputs.push_back({ "flow", "flow" });
		reg->RegisterNode("Blueprint", "End", std::move(end));
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
