#include "GraphView.h"
#include"ArcApp.h"
#include<ArcRx/ImageManager.h>
#include<ArcLog/LogSystem.h>
#include<Hash.h>
#include<chrono>
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
        BGTextureTileing = 4;
        bTickAllowed = true;
    }

    void GraphView::AddNode(std::shared_ptr<GraphNode> node)
    {
        if (!node)
        {
            return;
        }

        node->NodeId = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
        hash_combine(node->NodeId, ++NodeIdCounter);

        Nodes[node->NodeId] = node;
        AddChild(node); 
    }

    void GraphView::RemoveNode(size_t id)
    {
        auto n = Nodes.find(id);
        if (n != Nodes.end())
        {
            n->second->DisconnectAll();
            PushEvent({ [this,id] {
                Nodes.erase(id); 
            } });
        }
    }

    void GraphView::AddConnection(std::shared_ptr<GraphConnection>&& conn)
    {
        if (!conn)
        {
            return;
        }
        conn->Connect();
        Connections[conn->ConnectionId] = std::move(conn);
    }

    void GraphView::RemoveConnection(size_t connId)
    {
        auto iter = Connections.find(connId);
        if (iter == Connections.end())
        {
            return;
        }

        iter->second->Disconnect();
        Connections.erase(iter);
    }

    ImVec2 GraphView::ScreenToGraph(const ImVec2& screenPos) const
    {
        return (screenPos - AbsoluteRect.Min) / Zoom - ScrollOffset;
    }

    ImVec2 GraphView::GraphToScreen(const ImVec2& graphPos) const
    {
        return (graphPos + ScrollOffset) * Zoom + AbsoluteRect.Min;
    }

    ImVec2 GraphView::GetCanvasOrigin() const
    {
        return GraphToScreen({ 0.f,0.f });
    }

    void GraphView::Draw()
    {
        //handle context menu
       
        ImGui::PushID(this);
       
        ImGui::SetCursorScreenPos(AbsoluteRect.Min + Position);
        if (!ImGui::BeginChild("GraphView", Size, ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImGui::PopID();
            return;
        }

        DrawBackgroundGrid();
        DrawConnections();
        DrawNodes();

        if (ImGui::BeginPopupContextWindow("GraphCtxMenu",ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_MouseButtonDefault_))
        {
            DrawContextMenu();
            ImGui::EndPopup();
        }
        ImGui::EndChild();
       
        ImGui::PopID();
       
       
    }

    void GraphView::Tick(double dt)
    {
        //dispatch pending events
        mEventDispatcher.DispatchEvents(dt);
        Widget::Tick(dt);
    }

    void GraphView::DrawContextMenu()
    {
       //here add graph specific commands
    }

    void GraphView::DrawNodes()
    {
        // nodes draw themselves; ensure nodes' Draw() called in order (topmost last if you want top Z)
        for (auto& [id,n] : Nodes)
        {
            if (n->IsVisible())
                n->Draw();
        }
    }

    void GraphView::DrawConnections()
    {
        for (const auto& [id, conn] : Connections)
        {
            DrawConnection(*conn);
        }
    }

    void GraphView::DrawConnection(const GraphConnection& conn)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        auto f = conn.From.lock();
        auto t = conn.To.lock();
        if (!f || !t) return;
        ImVec2 p1 = f->GetAbsoluteRect().Center();
        ImVec2 p2 = t->GetAbsoluteRect().Center();
        ImVec2 c1 = ImVec2(p1.x + 60.0f, p1.y);
        ImVec2 c2 = ImVec2(p2.x - 60.0f, p2.y);
        dl->AddBezierCubic(p1, c1, c2, p2, IM_COL32(120, 160, 255, 255), 3.0f);
    }

    void GraphView::DrawBackgroundGrid()
    {
        if (!Tex_BG)
            return;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 tiling(BGTextureTileing / Tex_BG->GetWidth(), BGTextureTileing / Tex_BG->GetHeight());

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
   
    //node dragging logic
    void GraphView::BeginDragNode(std::shared_ptr<GraphNode> n)
    {
        if (DraggedNode)
        {
            EndDragNode();
        }
        DraggedNode = std::move(n);
    }

    void GraphView::UpdateDragNode(const ImVec2& mouseDelta)
    {
        if (DraggedNode)
        {
            DraggedNode->GraphPos += mouseDelta / Zoom;
        }
    }

    void GraphView::EndDragNode()
    {
        DraggedNode = nullptr;
    }

    bool GraphView::OnMouseWheel(WidgetEvent& e)
    {
        ImVec2 OldMouseGraphPos = ScreenToGraph(e.MousePos);
        Zoom = std::clamp(Zoom + e.Value.x * 0.2f, ZoomRange.x, ZoomRange.y);
        //adjust to keep mouse if correct position in graph
        ScrollOffset += ScreenToGraph(e.MousePos) - OldMouseGraphPos;
        return true;
    }

    bool GraphView::OnMouseMove(WidgetEvent& e)
    {
        if (DraggedNode)
        {
            UpdateDragNode(e.Value);
            return true;
        }

        //panning
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            ScrollOffset += e.Value / Zoom; //zoom aware scrolling
            return true;
        }
        return false;
    }

    void GraphView::SetFocusedNode(std::weak_ptr<GraphNode> n)
    {
        FocusedNode = n;
        //TODO: check for actual change before broadcasting
        OnFocusedNodeChanged(std::move(n));
    }

    void GraphView::PushEvent(ARC::Event&& e)
    {
        mEventDispatcher.PushEvent(std::move(e));
    }

    bool GraphView::OnMouseClick(WidgetEvent& e)
    {
        return false;
    }

    bool GraphView::OnMouseRelease(WidgetEvent& e)
    {
        if (DraggedNode && e.Key == ImGuiMouseButton_Left)
        {
            EndDragNode();
            return true;
        }
        return false;
    }
}