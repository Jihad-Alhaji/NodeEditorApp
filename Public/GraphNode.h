#pragma once
#include "GraphPin.h"
#include "UI_VerticalBox.h"
#include <vector>
#include <memory>
#include <nlohmann.hpp>
namespace NodeEditor
{
class GraphNode : public Widget, public std::enable_shared_from_this<GraphNode>
{
public:
    std::string Title;
    ImVec2 GraphPos = { 0, 0 }; // position in graph space (not screen)
    ImVec2 NodeSize = { 160, 80 };// original node size
    size_t NodeId = -1;
    uint64_t DocumentId = 0;
    std::string TypeKey;
    virtual nlohmann::json SaveProperties() const { return nlohmann::json::object(); }
    virtual bool LoadProperties(const nlohmann::json& value, std::string& diagnostic) { if (!value.is_object() || !value.empty()) { diagnostic = "unsupported node properties"; return false; } return true; }

    std::vector<std::shared_ptr<GraphPin>> Inputs;
    std::vector<std::shared_ptr<GraphPin>> Outputs;

    
    GraphNode(const std::string& title);

    //custom layout based on graph layout
    void UpdateLayout(const Rect& ParentRect)override;

    virtual void DrawContextMenu();
    virtual void DrawContents() {}

    // widget overrides
    virtual void Draw() override;
    virtual bool OnMouseClick(WidgetEvent& e) override;
    virtual bool OnMouseRelease(WidgetEvent& e) override;
    virtual bool OnMouseMove(WidgetEvent& e) override;
    virtual bool OnFocusGained(WidgetEvent& Even)override;//TODO: notify graph that this node is focused

    inline class GraphView* GetGraph()const;

    // convenience to add pins
    void AddPin(std::shared_ptr<GraphPin> pin);
   
    //utilities
    void DisconnectAll();
    void DeleteNode();
    void DublicateNode();
protected:
    std::shared_ptr<UI_VerticalBox> VB_InPins;
    std::shared_ptr<UI_VerticalBox> VB_OutPins;
};

}
