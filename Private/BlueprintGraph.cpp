#include "BlueprintGraph.h"
#include "BlueprintNode.h"
#include "BlueprintNodes.h"
#include "NodeEditorRegistery.h"
#include <imgui/imgui.h>
#include <ArcLog/LogSystem.h>
#include <algorithm>
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

BlueprintGraph::ExecutionResult BlueprintGraph::Execute(std::size_t stepLimit)
{
	using namespace NodeEditor;
	ExecutionResult result;
	auto fail = [&result](const std::string& message) { result.Diagnostic = message; return result; };
	std::vector<std::shared_ptr<BlueprintNode>> starts;
	std::unordered_set<const GraphNode*> graphNodes;
	std::unordered_map<const GraphNode*, std::shared_ptr<BlueprintNode>> blueprintNodes;
	for (const auto& [id, node] : Nodes)
	{
		if (!node) return fail("Graph contains a null node entry (ID " + std::to_string(id) + ").");
		graphNodes.insert(node.get());
		auto blueprint = std::dynamic_pointer_cast<BlueprintNode>(node);
		if (blueprint) { blueprintNodes[node.get()] = blueprint; if (blueprint->IsStartNode()) starts.push_back(blueprint); }
	}
	if (starts.empty()) return fail("Graph must contain at least one Start node; found 0.");
	std::sort(starts.begin(), starts.end(), [](const auto& a, const auto& b) { return a->NodeId < b->NodeId; });
	result.StartResults.reserve(starts.size());

	struct FlowEdge { std::shared_ptr<GraphPin> From; std::shared_ptr<GraphPin> To; };
	std::unordered_map<const GraphPin*, FlowEdge> flowByOutput;
	std::unordered_map<const GraphNode*, std::size_t> incoming;
	std::unordered_set<const GraphConnection*> graphConnections;
	std::unordered_map<const GraphNode*, std::unordered_set<std::string>> pinKeys;
	for (const auto& [nodeId, node] : Nodes)
	{
		for (const auto* pins : { &node->Inputs, &node->Outputs }) for (const auto& pin : *pins)
		{
			if (!pin) return fail("Node " + node->Title + " contains a null pin.");
			if (pin->PinKey.empty() || !pinKeys[node.get()].insert(pin->PinKey).second)
				return fail("Node " + node->Title + " (ID " + std::to_string(nodeId) + ") has an empty or duplicate pin key.");
			if (pin->ParentNode.lock() != node) return fail("Pin " + pin->PinKey + " on node " + node->Title + " has invalid ownership.");
			if ((pin->Role == EPinRole::Flow) != pin->IsFlowPin()) return fail("Pin " + pin->PinKey + " on node " + node->Title + " has invalid role metadata.");
		}
	}
	auto validateConnection = [&](const std::shared_ptr<GraphConnection>& connection, std::size_t id) -> std::string {
		if (!connection) return "Graph contains a null connection entry (ID " + std::to_string(id) + ").";
		auto from = connection->From.lock(); auto to = connection->To.lock();
		if (!from || !to) return "Connection " + std::to_string(id) + " has an expired endpoint.";
		auto source = from->ParentNode.lock(); auto destination = to->ParentNode.lock();
		if (!source || !destination || !graphNodes.count(source.get()) || !graphNodes.count(destination.get())) return "Connection " + std::to_string(id) + " has an endpoint outside this graph.";
		if (source == destination) return "Connection " + std::to_string(id) + " links a node to itself.";
		if (from->Type != EPinType::Output || to->Type != EPinType::Input) return "Connection " + std::to_string(id) + " has invalid direction; links must run output to input.";
		if (std::find(source->Outputs.begin(), source->Outputs.end(), from) == source->Outputs.end() || std::find(destination->Inputs.begin(), destination->Inputs.end(), to) == destination->Inputs.end()) return "Connection " + std::to_string(id) + " references a pin not owned by its endpoint node.";
		if (!from->AllowConnectionWith(to) || !to->AllowConnectionWith(from)) return "Connection " + std::to_string(id) + " is rejected by endpoint compatibility policy.";
		if (from->Role != to->Role) return "Connection " + std::to_string(id) + " has mismatched pin roles.";
		auto fr = from->Connections.find(id); auto tr = to->Connections.find(id);
		if (fr == from->Connections.end() || tr == to->Connections.end() || fr->second.lock() != connection || tr->second.lock() != connection) return "Connection " + std::to_string(id) + " is missing or mismatched in an endpoint pin.";
		if (from->Role == EPinRole::Flow)
		{
			if (!from->IsFlowPin() || !to->IsFlowPin()) return "Connection " + std::to_string(id) + " has invalid flow pin metadata.";
			if (!blueprintNodes.count(source.get()) || !blueprintNodes.count(destination.get())) return "Connection " + std::to_string(id) + " links a flow pin on a node that does not implement Blueprint execution.";
			if (from->AllowMultipleConnections()) return "Flow output " + from->PinKey + " on node " + source->Title + " must allow at most one connection.";
			if (flowByOutput.count(from.get())) return "Flow output " + from->PinKey + " on node " + source->Title + " has multiple outgoing links.";
			flowByOutput.emplace(from.get(), FlowEdge{ from, to }); ++incoming[destination.get()];
		}
		return {};
	};
	for (const auto& [id, connection] : Connections)
	{
		auto error = validateConnection(connection, id); if (!error.empty()) return fail(error);
		graphConnections.insert(connection.get());
	}
	for (const auto& [nodeId, node] : Nodes) for (const auto* pins : { &node->Inputs, &node->Outputs }) for (const auto& pin : *pins)
	{
		for (const auto& [id, weak] : pin->Connections)
		{
			auto connection = weak.lock();
			if (!connection || !graphConnections.count(connection.get()) || !Connections.count(id) || Connections.at(id) != connection) return fail("Pin " + pin->PinKey + " on node " + node->Title + " has an invalid connection reference (ID " + std::to_string(id) + ").");
			auto endpoint = pin->Type == EPinType::Input ? connection->To.lock() : connection->From.lock();
			if (endpoint != pin) return fail("Pin " + pin->PinKey + " on node " + node->Title + " has a mismatched connection reference (ID " + std::to_string(id) + ").");
		}
		if (!pin->AllowMultipleConnections() && pin->Connections.size() > 1) return fail("Pin " + pin->PinKey + " on node " + node->Title + " exceeds its connection cardinality.");
	}
	for (const auto& start : starts)
	{
		std::size_t outCount = 0;
		for (const auto& pin : start->Outputs) if (pin->Role == EPinRole::Flow) { auto f = flowByOutput.find(pin.get()); if (f != flowByOutput.end()) ++outCount; }
		if (incoming[start.get()] != 0) return fail("Start node " + start->Title + " (ID " + std::to_string(start->NodeId) + ") must not have incoming flow links.");
		if (outCount != 1) return fail("Start node " + start->Title + " (ID " + std::to_string(start->NodeId) + ") must have exactly one outgoing flow link.");
	}
	for (const auto& [nodePtr, blueprint] : blueprintNodes) if (blueprint->IsEndNode())
		for (const auto& pin : blueprint->Outputs) if (pin->Role == EPinRole::Flow && flowByOutput.count(pin.get())) return fail("End node " + blueprint->Title + " has an outgoing flow link.");

	for (const auto& start : starts) { ExecutionResult::StartResult sr; sr.StartNodeId = start->NodeId; result.StartResults.push_back(std::move(sr)); }
	for (std::size_t i = 0; i < starts.size(); ++i)
	{
		auto& sr = result.StartResults[i]; auto current = starts[i]; std::unordered_set<const GraphNode*> visited;
		while (true)
		{
			if (sr.StepsExecuted >= stepLimit) { sr.Diagnostic = "Execution step limit of " + std::to_string(stepLimit) + " exceeded."; break; }
			if (!visited.insert(current.get()).second) { sr.Diagnostic = "Cycle detected at node " + current->Title + " (ID " + std::to_string(current->NodeId) + ")."; break; }
			++sr.StepsExecuted; ++result.StepsExecuted; sr.Executed = true;
			if (current->IsEndNode()) { sr.Success = true; sr.Diagnostic = "Execution completed successfully."; break; }
			auto nodeResult = current->ExecuteFlow();
			if (nodeResult.Result == BlueprintNode::ExecutionResult::Outcome::Failure)
			{
				sr.Diagnostic = nodeResult.Diagnostic.empty() ? "Node execution failed: " + current->Title + " (ID " + std::to_string(current->NodeId) + ")." : nodeResult.Diagnostic;
				break;
			}
			if (nodeResult.Result == BlueprintNode::ExecutionResult::Outcome::Complete)
			{
				sr.Success = true; sr.Diagnostic = "Execution completed successfully."; break;
			}
			std::shared_ptr<GraphPin> selected;
			for (const auto& pin : current->Outputs) if (pin->Role == EPinRole::Flow && pin->PinKey == nodeResult.OutputPinKey) { selected = pin; break; }
			if (!selected) { sr.Diagnostic = "Node " + current->Title + " selected invalid flow output key '" + nodeResult.OutputPinKey + "'."; break; }
			auto edge = flowByOutput.find(selected.get());
			if (edge == flowByOutput.end()) { sr.Diagnostic = "Node " + current->Title + " selected unconnected flow output '" + nodeResult.OutputPinKey + "'."; break; }
			auto dest = edge->second.To->ParentNode.lock(); auto next = dest ? blueprintNodes.find(dest.get()) : blueprintNodes.end();
			if (next == blueprintNodes.end()) { sr.Diagnostic = "Flow output '" + nodeResult.OutputPinKey + "' reaches a destination that does not implement Blueprint execution."; break; }
			current = next->second;
		}
		if (!sr.Success) { result.Diagnostic = "Start node " + std::to_string(sr.StartNodeId) + " failed: " + sr.Diagnostic; return result; }
	}
	result.Success = true; result.Diagnostic = "Execution completed successfully."; return result;
}
