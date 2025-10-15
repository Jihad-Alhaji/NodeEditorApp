#pragma once
#include"Widget.h"
#include<Graphics/SimpleTexture.h>
#include <vector>
#include <memory>
#include"GraphNode.h"
#include<ArcCore/Structures/Delegate.h>

namespace NodeEditor
{

    class GraphView : public Widget
    {
    public:
        GraphView();

        virtual void Draw() override;
       

        // coordinate conversion
        ImVec2 ScreenToGraph(const ImVec2& screenPos) const;
        ImVec2 GraphToScreen(const ImVec2& graphPos) const;

        // nodes & connections management
        void AddNode(std::shared_ptr<GraphNode> node);
        void AddConnection(const GraphConnection& conn);

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
    protected:
        // original fields
        std::shared_ptr<Graphics::SimpleTexture> Tex_BG;
        ImVec2 ScrollOffset;
        ImVec2 ZoomRange;
        float Zoom;
        float BGTextureTileing;

        // nodes and connections
        std::vector<std::shared_ptr<GraphNode>> Nodes;
        std::vector<GraphConnection> Connections;

        std::shared_ptr<GraphNode> DraggedNode;

        std::weak_ptr<GraphNode> FocusedNode;
        // drawing helpers
        void DrawBackgroundGrid();
        void DrawConnections();
        void DrawNodes();
        ImVec2 GetCanvasOrigin() const;


        virtual void DrawConnection(const GraphConnection& con);
        virtual void DrawContextMenu();
    };
}