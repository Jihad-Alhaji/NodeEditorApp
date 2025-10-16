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

    inline bool IsConnected()const { return !Connections.empty(); }
    void DisconnectAll();

    //effectively its index in the parent node
    uint8_t PinId = -1;

    std::unordered_map<size_t, std::weak_ptr<class GraphConnection>> Connections;

  
};

class GraphConnection : public std::enable_shared_from_this<GraphConnection>
{
public:
    GraphConnection(std::shared_ptr<GraphPin> from, std::shared_ptr<GraphPin> to);
   
    std::weak_ptr<GraphPin> From;
    std::weak_ptr<GraphPin> To;

    // generated from pin and node ids
    size_t ConnectionId = -1;

protected:
    friend class GraphView;
    void Connect();
    void Disconnect();
};

class GraphNode : public Widget, public std::enable_shared_from_this<GraphNode>
{
public:
    std::string Title;
    ImVec2 GraphPos = { 0, 0 }; // position in graph space (not screen)
    ImVec2 NodeSize = { 160, 80 };// original node size
    size_t NodeId = -1;

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
   
    //utilities
    void DisconnectAll();
    void DeleteNode();
    void DublicateNode();
protected:
    std::shared_ptr<UI_VerticalBox> VB_InPins;
    std::shared_ptr<UI_VerticalBox> VB_OutPins;
};
}