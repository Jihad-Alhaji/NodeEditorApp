#pragma once
#include "GraphView.h"
#include "NodeGraphFactories.h"
#include <string>
#include <vector>
#include <cstddef>
#include <memory>
class BlueprintGraph : public NodeEditor::GraphView
{
public:
	struct ExecutionResult
	{
		struct StartResult
		{
			size_t StartNodeId = 0;
			bool Executed = false;
			bool Success = false;
			std::size_t StepsExecuted = 0;
			std::string Diagnostic;
		};
		bool Success = false;
		std::string Diagnostic;
		std::size_t StepsExecuted = 0;
		std::vector<StartResult> StartResults;
	};
	static constexpr std::size_t DefaultStepLimit = 1024;
	BlueprintGraph();
	ExecutionResult Execute(std::size_t stepLimit = DefaultStepLimit);
protected:
    void DrawContextMenu() override;
    std::vector<std::string> ctx_Nodes;
};
struct BlueprintGraph_Factory : public GraphViewFactory
{
    std::shared_ptr<NodeEditor::GraphView> Create(class NodeEditorRegistery*) const override { return std::make_shared<BlueprintGraph>(); }
};
