#include "GraphView.h"
#include"ArcApp.h"
#include<ArcRx/ImageManager.h>
#include<ArcLog/LogSystem.h>
namespace NodeEditor
{

    GraphView::GraphView() :Widget("GraphView")
    {
        auto* app = ARC::App::ArcApp::GetInstance();
        Tex_BG = Graphics::ResourceTexManager::GetOrCreate(app->GetResourcesDir() + "Textures/grid.png");

        Position = { 0, 0 };
        Size = { 800, 600 };
        ScrollOffset = { 0, 0 };
        Zoom = 1.f;
        ZoomRange = { 0.6f, 1.8f };
        bTickAllowed = true;
        BGTextureTileing = 4;
        DraggingFromPin = nullptr;
    }

    void GraphView::Construct()
    {
    }

    void GraphView::Tick(double DeltaTime)
    {
    }

    void GraphView::AddNode(std::shared_ptr<GraphNode> node)
    {
        Nodes.push_back(node);
        AddChild(node); // so FindWidgetAt works via widget tree
    }

    void GraphView::AddConnection(const GraphConnection& conn)
    {
        Connections.push_back(conn);
    }

    ImVec2 GraphView::ScreenToGraph(const ImVec2& screenPos) const
    {
        return (screenPos - AbsoluteRect.Min) / Zoom - ScrollOffset;
    }

    ImVec2 GraphView::GraphToScreen(const ImVec2& graphPos) const
    {
        return (graphPos + ScrollOffset) * Zoom + AbsoluteRect.Min;
    }

    void GraphView::Draw()
    {
        ImGui::PushID(this);
        ImGui::SetCursorScreenPos(AbsoluteRect.Min + Position);
        if (!ImGui::BeginChild("GraphView", Size, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))return;

        DrawBackgroundGrid();
        DrawConnections();
        DrawNodes();

         // draw dragging connection (temporary)
        if (DraggingFromPin)
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p1 = DraggingFromPin->GetScreenPos(this);
            ImVec2 p2 = DragMousePos;
            ImVec2 c1 = ImVec2(p1.x + 60.0f, p1.y);
            ImVec2 c2 = ImVec2(p2.x - 60.0f, p2.y);
            dl->AddBezierCubic(p1, c1, c2, p2, IM_COL32(200, 200, 100, 255), 3.0f);
        }

        ImGui::EndChild();
        ImGui::PopID();
    }

    void GraphView::DrawNodes()
    {
        // nodes draw themselves; ensure nodes' Draw() called in order (topmost last if you want top Z)
        for (auto& n : Nodes)
        {
            if (n->IsVisible())
                n->Draw();
        }
    }

    void GraphView::DrawConnections()
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        for (auto& conn : Connections)
        {
            auto f = conn.From.lock();
            auto t = conn.To.lock();
            if (!f || !t) continue;
            ImVec2 p1 = f->GetScreenPos(this);
            ImVec2 p2 = t->GetScreenPos(this);
            ImVec2 c1 = ImVec2(p1.x + 60.0f, p1.y);
            ImVec2 c2 = ImVec2(p2.x - 60.0f, p2.y);
            dl->AddBezierCubic(p1, c1, c2, p2, IM_COL32(120, 160, 255, 255), 3.0f);
        }
    }

    void GraphView::BeginLinkFromPin(std::shared_ptr<GraphPin> pin)
    {
        DraggingFromPin = pin;
        // capture initial mouse pos; UIManager will call UpdateLinkDrag each MouseMove
    }

    void GraphView::UpdateLinkDrag(const ImVec2& mouseScreenPos)
    {
        DragMousePos = mouseScreenPos;
    }

    void GraphView::TryCompleteLink(std::shared_ptr<GraphPin> targetPin)
    {
        if (!DraggingFromPin) return;
        // only allow connecting input->output or output->input; prevent same side
        if (DraggingFromPin->Type == targetPin->Type) return;

        GraphConnection c;
        if (DraggingFromPin->Type == EPinType::Output)
        {
            c.From = DraggingFromPin;
            c.To = targetPin;
        }
        else
        {
            c.From = targetPin;
            c.To = DraggingFromPin;
        }
        AddConnection(c);
        DraggingFromPin = nullptr;
    }

    bool GraphView::OnMouseWheel(WidgetEvent& e)
    {
       
        ImVec2 OldMouseGraphPos = ScreenToGraph(e.MousePos);
        Zoom = std::clamp(Zoom + e.Value.x * 0.2f, ZoomRange.x, ZoomRange.y);
        ScrollOffset += ScreenToGraph(e.MousePos) - OldMouseGraphPos;
        return true;
    }

    bool GraphView::OnMouseMove(WidgetEvent& e)
    {
        if (DraggingFromPin)
        {
            UpdateLinkDrag(e.MousePos);
            return true;
        }
        // pan if right mouse down and move
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            ScrollOffset += e.Value / Zoom; //zoom aware scrolling
            return true;
        }
        return false;
    }

    bool GraphView::OnMouseClick(WidgetEvent& e)
    {
        // if clicking empty space and we had a dragging link, cancel it
        if (DraggingFromPin && e.Key == ImGuiMouseButton_Left)
        {
            DraggingFromPin = nullptr;
            return true;
        }
        return false;
    }

    bool GraphView::OnMouseRelease(WidgetEvent& e)
    {
        // if releasing while dragging link, attempt to complete using hovered widget/pin
        if (DraggingFromPin && e.Key == ImGuiMouseButton_Left)
        {
            // UIManager DispatchEvent already determined Hit widget and called OnEvent
            // pinned target handling is done in GraphNode::OnMouseClick when clicked on pin
            // fallback: cancel
            DraggingFromPin = nullptr;
            return true;
        }
        return false;
    }

    void GraphView::DrawBackgroundGrid()
    {
        if (!Tex_BG)
            return;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 tiling (BGTextureTileing / Tex_BG->GetWidth() , BGTextureTileing / Tex_BG->GetHeight() );
        
        // Draw tiled background texture that scrolls and zooms with the view
        ImVec2 uv0 = ScrollOffset * tiling;
        ImVec2 uv1 = (ScrollOffset - Size / Zoom) * tiling;

        drawList->AddImage(
            (ImTextureID)Tex_BG->GetTexID(),
            AbsoluteRect.Min,
            AbsoluteRect.Max,
            uv0,
            uv1
        );
    }

    ImVec2 GraphView::GetCanvasOrigin() const
    {
        return GraphToScreen({ 0.f,0.f });
    }
}