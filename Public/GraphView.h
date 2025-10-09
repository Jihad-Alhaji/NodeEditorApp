#pragma once
#include"Widget.h"
#include<Graphics/SimpleTexture.h>
#include <vector>
#include <memory>
#include"GraphNode.h"
namespace NodeEditor
{

    class GraphView : public Widget
    {
    public:
        GraphView();

        virtual void Construct() override;
        virtual void Tick(double DeltaTime) override;
        virtual void Draw() override;
        
        // coordinate conversion
        ImVec2 ScreenToGraph(const ImVec2& screenPos) const;
        ImVec2 GraphToScreen(const ImVec2& graphPos) const;

        // nodes & connections management
        void AddNode(std::shared_ptr<GraphNode> node);
        void AddConnection(const GraphConnection& conn);

        // link dragging API for nodes to call
        void BeginLinkFromPin(std::shared_ptr<GraphPin> pin);
        void UpdateLinkDrag(const ImVec2& mouseScreenPos);
        void TryCompleteLink(std::shared_ptr<GraphPin> targetPin);

        bool OnMouseWheel(WidgetEvent& e)override;
        bool OnMouseClick(WidgetEvent& e)override;
        bool OnMouseRelease(WidgetEvent& e)override;
        bool OnMouseMove(WidgetEvent& e)override;
    protected:
        // original fields
        std::shared_ptr<Graphics::SimpleTexture> Tex_BG;
        ImVec2 ScrollOffset;
        float Zoom;

        ImVec2 ZoomRange;

        // nodes and connections
        std::vector<std::shared_ptr<GraphNode>> Nodes;
        std::vector<GraphConnection> Connections;

        // link dragging state
        std::shared_ptr<GraphPin> DraggingFromPin;
        ImVec2 DragMousePos;

        // drawing helpers
        void DrawBackgroundGrid();
        void DrawConnections();
        void DrawNodes();
        ImVec2 GetCanvasOrigin() const;
    };
}