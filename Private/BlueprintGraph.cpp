#include "BlueprintGraph.h"
#include"NodeEditorRegistery.h"
#include<imgui/imgui.h>
#include<ArcLog/LogSystem.h>
#include<imgui/imgui_stdlib.h>
#include <unordered_map>
#include <unordered_set>

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
		auto result = Execute();
		Logging::LogSystem::ArcLogger.Send(result.Success ? Logging::LogVerbosity::NOTE : Logging::LogVerbosity::ARCERROR,
			result.Success ? "Blueprint execution succeeded." : result.Diagnostic);
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

BlueprintFlowPin::BlueprintFlowPin()
{
	Role = NodeEditor::EPinRole::Flow;
}

BlueprintStartNode::BlueprintStartNode() : BlueprintNode()
{
	Title = "Start";
	NodeSize = { 160, 80 };
	SetSize(NodeSize);
}

BlueprintEndNode::BlueprintEndNode() : BlueprintNode()
{
	Title = "End";
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
	auto flowOut = std::make_shared<BlueprintFlowPin>();
	flowOut->SetName("flow");
	flowOut->Type = NodeEditor::EPinType::Output;
	node->AddPin(std::move(flowOut));
	return node;
}

std::shared_ptr<NodeEditor::GraphNode> BlueprintEndNode_Factory::Create(NodeEditorRegistery*) const
{
	auto node = std::make_shared<BlueprintEndNode>();
	auto flowIn = std::make_shared<BlueprintFlowPin>();
	flowIn->SetName("flow");
	flowIn->Type = NodeEditor::EPinType::Input;
	node->AddPin(std::move(flowIn));
	return node;
}

std::shared_ptr<NodeEditor::GraphNode> BlueprintPrintMessageNode_Factory::Create(NodeEditorRegistery* Registery) const
{
	auto node = std::make_shared<BlueprintPrintMessageNode>();
	for (const auto& direction : { NodeEditor::EPinType::Input, NodeEditor::EPinType::Output })
	{
		auto pin = std::make_shared<BlueprintFlowPin>();
		pin->SetName("flow");
		pin->Type = direction;
		node->AddPin(std::move(pin));
	}
	return node;
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

BlueprintGraph::ExecutionResult BlueprintGraph::Execute(std::size_t stepLimit)
{
	using namespace NodeEditor;
	ExecutionResult result;
	auto fail = [&result](const std::string& message) {
		result.Diagnostic = message;
		return result;
	};

	std::vector<std::shared_ptr<BlueprintNode>> starts;
	std::unordered_set<const GraphNode*> graphNodes;
	for (const auto& [id, node] : Nodes)
	{
		if (!node) return fail("Graph contains a null node entry (ID " + std::to_string(id) + ").");
		graphNodes.insert(node.get());
		auto blueprintNode = std::dynamic_pointer_cast<BlueprintNode>(node);
		if (blueprintNode && blueprintNode->IsStartNode()) starts.push_back(std::move(blueprintNode));
	}
	if (starts.size() != 1) return fail("Graph must contain exactly one Start node; found " + std::to_string(starts.size()) + ".");

	struct FlowEdge { std::shared_ptr<GraphPin> From; std::shared_ptr<GraphPin> To; };
	std::unordered_map<const GraphNode*, std::vector<FlowEdge>> outgoing;
	std::unordered_map<const GraphNode*, std::size_t> incoming;
	std::unordered_set<const GraphConnection*> graphConnections;
	auto validateConnection = [&](const std::shared_ptr<GraphConnection>& connection, std::size_t id) -> std::string {
		if (!connection) return "Graph contains a null connection entry (ID " + std::to_string(id) + ").";
		auto from = connection->From.lock();
		auto to = connection->To.lock();
		if (!from || !to) return "Connection " + std::to_string(id) + " has an expired endpoint.";
		auto sourceNode = from->ParentNode.lock();
		auto destinationNode = to->ParentNode.lock();
		if (!sourceNode || !destinationNode || !graphNodes.count(sourceNode.get()) || !graphNodes.count(destinationNode.get()))
			return "Connection " + std::to_string(id) + " has an endpoint outside this graph.";
		if (sourceNode == destinationNode) return "Connection " + std::to_string(id) + " links a node to itself.";
		if (from->Type != EPinType::Output || to->Type != EPinType::Input)
			return "Connection " + std::to_string(id) + " has invalid direction; links must run output to input.";
		if (std::find(sourceNode->Outputs.begin(), sourceNode->Outputs.end(), from) == sourceNode->Outputs.end() ||
			std::find(destinationNode->Inputs.begin(), destinationNode->Inputs.end(), to) == destinationNode->Inputs.end())
			return "Connection " + std::to_string(id) + " references a pin not owned by its endpoint node.";
		if (from->Role != to->Role) return "Connection " + std::to_string(id) + " has mismatched pin roles.";
		auto fromRef = from->Connections.find(id);
		auto toRef = to->Connections.find(id);
		if (fromRef == from->Connections.end() || toRef == to->Connections.end() ||
			fromRef->second.lock() != connection || toRef->second.lock() != connection)
			return "Connection " + std::to_string(id) + " is missing or mismatched in an endpoint pin.";
		if (from->Role == EPinRole::Data) return {};
		if (!from->IsFlowPin() || !to->IsFlowPin()) return "Connection " + std::to_string(id) + " has invalid flow pin metadata.";
		auto sourceBlueprintNode = std::dynamic_pointer_cast<BlueprintNode>(sourceNode);
		auto destinationBlueprintNode = std::dynamic_pointer_cast<BlueprintNode>(destinationNode);
		if (!sourceBlueprintNode || !destinationBlueprintNode || !sourceBlueprintNode->AllowsFlowOutput() || !destinationBlueprintNode->AllowsFlowInput())
			return "Connection " + std::to_string(id) + " violates a node's flow input/output behavior.";
		outgoing[sourceNode.get()].push_back({ from, to });
		++incoming[destinationNode.get()];
		return {};
	};

	for (const auto& [id, connection] : Connections)
	{
		auto error = validateConnection(connection, id);
		if (!error.empty()) return fail(error);
		graphConnections.insert(connection.get());
	}
	for (const auto& [nodeId, node] : Nodes)
	{
		for (const auto* pins : { &node->Inputs, &node->Outputs })
		{
			for (const auto& pin : *pins)
			{
				if (!pin) return fail("Node " + node->Title + " contains a null pin.");
				for (const auto& [id, weakConnection] : pin->Connections)
				{
					auto connection = weakConnection.lock();
					if (!connection || !graphConnections.count(connection.get()) || !Connections.count(id) || Connections.at(id) != connection)
						return fail("Pin on node " + node->Title + " has an invalid connection reference (ID " + std::to_string(id) + ").");
					auto endpoint = pin->Type == EPinType::Input ? connection->To.lock() : connection->From.lock();
					if (endpoint != pin) return fail("Pin on node " + node->Title + " has a mismatched connection reference (ID " + std::to_string(id) + ").");
				}
			}
		}
	}
	for (const auto& [source, edges] : outgoing)
		if (edges.size() > 1)
		{
			auto node = std::find_if(Nodes.begin(), Nodes.end(), [source](const auto& entry) { return entry.second.get() == source; });
			return fail("Node " + (node == Nodes.end() ? std::string("<unknown>") : node->second->Title) + " has multiple outgoing flow links.");
		}

	auto start = starts.front();
	if (incoming[start.get()] != 0) return fail("Start node must not have incoming flow links.");
	if (outgoing[start.get()].size() != 1) return fail("Start node must have exactly one outgoing flow link.");
	for (const auto& [nodePtr, edges] : outgoing)
	{
		auto nodeIt = std::find_if(Nodes.begin(), Nodes.end(), [nodePtr](const auto& entry) { return entry.second.get() == nodePtr; });
		if (nodeIt != Nodes.end())
		{
			auto node = std::dynamic_pointer_cast<BlueprintNode>(nodeIt->second);
			if (node && node->IsEndNode() && !edges.empty()) return fail("End node " + node->Title + " has an outgoing flow link.");
		}
	}

	std::vector<std::shared_ptr<BlueprintNode>> path;
	std::unordered_set<const GraphNode*> visited;
	auto current = start;
	while (true)
	{
		if (!visited.insert(current.get()).second) return fail("Execution path contains a cycle at node " + current->Title + ".");
		path.push_back(current);
		if (current->IsEndNode()) break;
		auto found = outgoing.find(current.get());
		if (found == outgoing.end() || found->second.empty()) return fail("Reachable node " + current->Title + " has no successor and does not terminate at End.");
		current = std::dynamic_pointer_cast<BlueprintNode>(found->second.front().To->ParentNode.lock());
		if (!current) return fail("Flow link destination does not implement Blueprint execution.");
	}
	if (path.size() > stepLimit) return fail("Execution step limit of " + std::to_string(stepLimit) + " would be exceeded.");

	for (const auto& node : path)
	{
		if (!node->Execute()) return fail("Node execution failed: " + node->Title + " (ID " + std::to_string(node->NodeId) + ").");
		++result.StepsExecuted;
	}
	result.Success = true;
	result.Diagnostic = "Execution completed successfully.";
	return result;
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
