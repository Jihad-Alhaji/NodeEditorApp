#pragma once
#include "Widget.h"
#include <string>
#include <vector>
#include <memory>

namespace NodeEditor
{


class GraphView; 

enum class EPinType { Input, Output };

struct GraphPin
{
    EPinType Type;
    std::string Name;
    // local position relative to node top-left
    ImVec2 LocalPos;

    // weak backref to owning node
    std::weak_ptr<class GraphNode> ParentNode;

    GraphPin(EPinType type, const std::string& name, ImVec2 localPos)
        : Type(type), Name(name), LocalPos(localPos) {
    }

    // screen position - requires node absolute rect (uses parent node)
    ImVec2 GetScreenPos(const GraphView* view) const;
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
    ImVec2 NodeSize = { 160, 80 };

    std::vector<std::shared_ptr<GraphPin>> Inputs;
    std::vector<std::shared_ptr<GraphPin>> Outputs;

    GraphNode(const std::string& title, const ImVec2& graphPos);

    void UpdateLayout(const Rect& ParentRect)override;

    // widget overrides
    virtual void Draw() override;
    virtual bool OnMouseClick(WidgetEvent& e) override;
    virtual bool OnMouseRelease(WidgetEvent& e) override;
    virtual bool OnMouseMove(WidgetEvent& e) override;

    // helper
    Rect GetRect() const; // in screen space (AbsoluteRect)
   
    inline class GraphView* GetGraph()const;

    // convenience to add pins
    std::shared_ptr<GraphPin> AddPin(EPinType type, const std::string& name, ImVec2 localPos);
};
}