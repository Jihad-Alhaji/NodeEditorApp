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

	//sort starts by id
	if (starts.empty()) return fail("Graph must contain at least one Start node; found 0.");
	std::sort(starts.begin(), starts.end(), [](const auto& a, const auto& b) { return a->NodeId < b->NodeId; });
	result.StartResults.reserve(starts.size());

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

	for (const auto& start : starts)
	{
		if (incoming[start.get()] != 0) return fail("Start node " + start->Title + " (ID " + std::to_string(start->NodeId) + ") must not have incoming flow links.");
		if (outgoing[start.get()].size() != 1) return fail("Start node " + start->Title + " (ID " + std::to_string(start->NodeId) + ") must have exactly one outgoing flow link.");
	}
	for (const auto& [nodePtr, edges] : outgoing)
	{
		auto nodeIt = std::find_if(Nodes.begin(), Nodes.end(), [nodePtr](const auto& entry) { return entry.second.get() == nodePtr; });
		if (nodeIt != Nodes.end())
		{
			auto node = std::dynamic_pointer_cast<BlueprintNode>(nodeIt->second);
			if (node && node->IsEndNode() && !edges.empty()) return fail("End node " + node->Title + " has an outgoing flow link.");
		}
	}

	std::vector<std::vector<std::shared_ptr<BlueprintNode>>> paths;
	paths.reserve(starts.size());
	for (const auto& start : starts)
	{
		std::vector<std::shared_ptr<BlueprintNode>> path;
		std::unordered_set<const GraphNode*> visited;
		auto current = start;
		while (true)
		{
			if (!visited.insert(current.get()).second) return fail("Start node " + start->Title + " (ID " + std::to_string(start->NodeId) + ") reaches a cycle at node " + current->Title + ".");
			path.push_back(current);
			if (current->IsEndNode()) break;
			auto found = outgoing.find(current.get());
			if (found == outgoing.end() || found->second.empty()) return fail("Start node " + start->Title + " (ID " + std::to_string(start->NodeId) + ") reaches node " + current->Title + " without a successor or End.");
			current = std::dynamic_pointer_cast<BlueprintNode>(found->second.front().To->ParentNode.lock());
			if (!current) return fail("Start node " + start->Title + " (ID " + std::to_string(start->NodeId) + ") reaches a destination that does not implement Blueprint execution.");
		}
		if (path.size() > stepLimit) return fail("Start node " + start->Title + " (ID " + std::to_string(start->NodeId) + ") would exceed the execution step limit of " + std::to_string(stepLimit) + ".");
		paths.push_back(std::move(path));
		ExecutionResult::StartResult startResult;
		startResult.StartNodeId = start->NodeId;
		result.StartResults.push_back(std::move(startResult));
	}

	for (std::size_t i = 0; i < paths.size(); ++i)
	{
		auto& startResult = result.StartResults[i];
		for (const auto& node : paths[i])
		{
			if (!node->Execute())
			{
				startResult.Diagnostic = "Node execution failed: " + node->Title + " (ID " + std::to_string(node->NodeId) + ").";
				result.Diagnostic = "Start node " + std::to_string(startResult.StartNodeId) + " failed: " + startResult.Diagnostic;
				return result;
			}
			++startResult.StepsExecuted;
			++result.StepsExecuted;
		}
		startResult.Executed = true;
		startResult.Success = true;
		startResult.Diagnostic = "Execution completed successfully.";
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
