#include "BlueprintGraph.h"
#include"NodeEditorRegistery.h"
#include<imgui/imgui.h>

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

void BlueprintNode::DrawContextMenu()
{
	GraphNode::DrawContextMenu();
	ImGui::Separator();
	if (ImGui::Selectable("Execute From Here"))
	{

	}
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



void RegisterBlueprintGraph()
{
	auto reg = NodeEditorRegistery::Get();
	reg->RegisterGraph("Blueprint", std::make_shared<BlueprintGraph_Factory>());

	reg->RegisterPin("default", std::make_shared<BlueprintPin_Default_Factory>());
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
