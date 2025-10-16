#pragma once
#include"Widget.h"
#include<Graphics/SimpleTexture.h>
#include <vector>
#include <memory>
#include"GraphNode.h"
#include<Structures/Delegate.h>
#include<Event.h>
namespace NodeEditor
{

    class GraphView : public Widget
    {
    public:
        GraphView();

        virtual void Draw() override;
        virtual void Tick(double dt)override;

        // coordinate conversion
        ImVec2 ScreenToGraph(const ImVec2& screenPos) const;
        ImVec2 GraphToScreen(const ImVec2& graphPos) const;

        // nodes & connections management
        void AddNode(std::shared_ptr<GraphNode> node);
        void RemoveNode(size_t id);

        void AddConnection(std::shared_ptr<GraphConnection>&& conn);
        void RemoveConnection(size_t connId);

        //node dragging logic
        void BeginDragNode(std::shared_ptr<GraphNode> n);
        void UpdateDragNode(const ImVec2& mouseDelta);
        void EndDragNode();

        bool OnMouseWheel(WidgetEvent& e)override;
        bool OnMouseClick(WidgetEvent& e)override;
        bool OnMouseRelease(WidgetEvent& e)override;
        bool OnMouseMove(WidgetEvent& e)override;

        inline float GetZoom()const { return Zoom; }

        void SetFocusedNode(std::weak_ptr<GraphNode> n);
        Structures::TMultiDelegate<std::weak_ptr<GraphNode>> OnFocusedNodeChanged;

        void PushEvent(ARC::Event&& e);

        
    protected:
        // original fields
        std::shared_ptr<Graphics::SimpleTexture> Tex_BG;
        ImVec2 ScrollOffset;
        ImVec2 ZoomRange;
        float Zoom;
        float BGTextureTileing;

        size_t NodeIdCounter = 0;

        // nodes and connections
        std::unordered_map<size_t, std::shared_ptr<GraphNode>> Nodes;
        
        std::unordered_map<size_t,std::shared_ptr<GraphConnection>> Connections;
     
        std::shared_ptr<GraphNode> DraggedNode;

        std::weak_ptr<GraphNode> FocusedNode;

        ARC::EventDispatcher mEventDispatcher;

        // drawing helpers
        void DrawBackgroundGrid();
        void DrawConnections();
        void DrawNodes();
        ImVec2 GetCanvasOrigin() const;


        virtual void DrawConnection(const GraphConnection& con);
        virtual void DrawContextMenu();
    };
}