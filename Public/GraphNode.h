#pragma once
#include "Widget.h"
#include <string>
#include <vector>
#include <memory>
#include"UI_VerticalBox.h"

namespace NodeEditor
{
class GraphView; 

enum class EPinType { Input, Output };

class GraphPin : public Widget, public std::enable_shared_from_this<GraphPin>
{
public:
    EPinType Type = EPinType::Input;

    // weak backref to owning node
    std::weak_ptr<class GraphNode> ParentNode;

    GraphPin();

    void Draw()override;
    virtual void DrawContextMenu();
    bool OnRecieveDrop(WidgetEvent& e) override;

    void SetName(std::string pinName) { Name = std::move(pinName); };
};

struct GraphConnection
{
    std::weak_ptr<GraphPin> From;
    std::weak_ptr<GraphPin> To;
};

class GraphNode : public Widget, public std::enable_shared_from_this<GraphNode>
{
public:
    std::string Title;
    ImVec2 GraphPos = { 0, 0 }; // position in graph space (not screen)
    ImVec2 NodeSize = { 160, 80 };// original node size

    std::vector<std::shared_ptr<GraphPin>> Inputs;
    std::vector<std::shared_ptr<GraphPin>> Outputs;

    GraphNode(const std::string& title);

    //custom layout based on graph layout
    void UpdateLayout(const Rect& ParentRect)override;

    virtual void DrawContextMenu();

    // widget overrides
    virtual void Draw() override;
    virtual bool OnMouseClick(WidgetEvent& e) override;
    virtual bool OnMouseRelease(WidgetEvent& e) override;
    virtual bool OnMouseMove(WidgetEvent& e) override;
    virtual bool OnFocusGained(WidgetEvent& Even)override;//TODO: notify graph that this node is focused

    inline class GraphView* GetGraph()const;

    // convenience to add pins
    void AddPin(std::shared_ptr<GraphPin> pin);

    
protected:
    std::shared_ptr<UI_VerticalBox> VB_InPins;
    std::shared_ptr<UI_VerticalBox> VB_OutPins;
};
}