#pragma once
#include "Widget.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <cstdint>
namespace NodeEditor
{
class GraphView; class GraphNode;
enum class EPinType { Input, Output };
enum class EPinRole { Data, Flow };

class GraphPin : public Widget, public std::enable_shared_from_this<GraphPin>
{
public:
    EPinType Type = EPinType::Input;
    EPinRole Role = EPinRole::Data;
	

    std::string Name;

    // weak backref to owning node
    std::weak_ptr<class GraphNode> ParentNode;

    GraphPin();
    virtual bool IsFlowPin() const { return Role == EPinRole::Flow; }
    virtual bool AllowMultipleConnections() const;
    virtual bool AllowConnectionWith(std::shared_ptr<GraphPin> other) const;
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


}


