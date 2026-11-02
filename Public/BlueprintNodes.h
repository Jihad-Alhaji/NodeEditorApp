#pragma once
#include "BlueprintNode.h"
#include "GraphPin.h"
#include <string>


class BlueprintStartNode : public BlueprintNode 
{ 
	public: BlueprintStartNode(); 
	bool Execute() override { return true; } 
	bool IsStartNode() const override { return true; } 
};

class BlueprintPrintMessageNode : public BlueprintNode
{
	public: BlueprintPrintMessageNode();
	 nlohmann::json SaveProperties() const override { return Message == "Hello from my workflow" ? nlohmann::json::object() : nlohmann::json{ { "message", Message } }; }
	 bool LoadProperties(const nlohmann::json& value, std::string& diagnostic) override { if (!value.is_object()) { diagnostic = "expected properties object"; return false; } if (value.empty()) return true; if (value.size() != 1 || !value.contains("message") || !value["message"].is_string()) { diagnostic = "properties.message must be a string"; return false; } Message = value["message"].get<std::string>(); return true; }
	std::string Message = "Hello from my workflow"; 
	bool Execute() override;
	protected: void DrawContents() override;
};

struct BlueprintStartNode_Factory : public GraphNodeFactory 
{ 
	std::string GetTypeKey() const override { return "Start"; }
	std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery*) const override; 
};

struct BlueprintPrintMessageNode_Factory : public GraphNodeFactory 
{ 
	std::string GetTypeKey() const override { return "Print Message"; }
	std::shared_ptr<NodeEditor::GraphNode> Create(class NodeEditorRegistery*) const override; 
};


