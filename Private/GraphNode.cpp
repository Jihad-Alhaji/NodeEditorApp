#include "GraphNode.h"
#include "GraphView.h"
#include <ArcLog/LogSystem.h>
namespace NodeEditor
{
    GraphNode::GraphNode(const std::string& title)
        : Widget("GraphNode"), Title(title)
    {
        Size = NodeSize;
        bTickAllowed = false;
        VB_InPins = std::make_shared<UI_VerticalBox>();
        VB_InPins->SetAlignment(EAlign::Left, EAlign::Center);
        VB_InPins->SetAutoSize(true);
        AddChild(VB_InPins);

        VB_OutPins = std::make_shared<UI_VerticalBox>();
        VB_OutPins->SetAlignment(EAlign::Right, EAlign::Center);
        VB_OutPins->SetAutoSize(true);
        AddChild(VB_OutPins);
    }

    void GraphNode::UpdateLayout(const Rect& ParentRect)
    {
        GraphView* view = GetGraph();
        if (!view) return;

        // compute screen position and set Position so UpdateLayout makes AbsoluteRect correct
        ImVec2 screen = view->GraphToScreen(GraphPos);

        // Position is relative to parent (GraphView's AbsoluteRect.Min). We'll set Position accordingly
        Position = screen - ParentRect.Min;
        Size = NodeSize * view->GetZoom();

        AbsoluteRect.Min = screen;
        AbsoluteRect.Max = screen + Size;

        // Propagate to children
        for (auto& Child : Children)
        {
            if (Child->IsVisible())
                Child->UpdateLayout(AbsoluteRect);
        }
    }

    GraphView* GraphNode::GetGraph() const
    {
        return dynamic_cast<GraphView*>(Parent);
    }

    void GraphNode::AddPin(std::shared_ptr<GraphPin> pin)
    {
        if (!pin)
            return;
        pin->ParentNode = shared_from_this();
        if (pin->Type == EPinType::Input)
        {
            VB_InPins->AddChild(pin);
            pin->PinId = static_cast<uint32_t>(Inputs.size());
            Inputs.emplace_back(std::move(pin));
        }
        else
        {
            VB_OutPins->AddChild(pin);
            pin->PinId = static_cast<uint32_t>(Outputs.size());
            Outputs.emplace_back(std::move(pin));
        }
    }
    
    void GraphNode::Draw()
    {
        ImGui::PushID(this);
       
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 a = AbsoluteRect.Min;
        ImVec2 b = AbsoluteRect.Max;
        
        //padding so pins appear outside a little
        a.x += 8;
        b.x -= 8;
        ImGui::SetCursorScreenPos(a);
        ImGui::InvisibleButton("btn", b-a);//hit box 
        if (ImGui::BeginPopupContextItem("NodeCtxMenu"))
        {
            DrawContextMenu();
            ImGui::EndPopup();
        }
        // Node background
        dl->AddRectFilled(a, b, IM_COL32(60, 60, 60, 220), 6.0f);
        
        // Header bar
        ImVec2 headerB = ImVec2(b.x, a.y + 22.0f);
        dl->AddRectFilled(a, headerB, IM_COL32(30, 30, 30, 220), 6.0f);

        // Title
        dl->AddText(ImVec2(a.x + 8.0f, a.y + 4.0f), IM_COL32_WHITE, Title.c_str());

        DrawContents();
        
        if (bFocused)
        {
            dl->AddRect(a, b, IM_COL32(220, 200, 20, 220), 6.f, 0, 2.f);
        }
        //draws children including pins
        Widget::Draw();
        
       
       
        ImGui::PopID();
        
    }

    void GraphNode::DrawContextMenu()
    {
        if (ImGui::Selectable("Delete"))
        {
            DeleteNode();
        }
        if (ImGui::Selectable("Dublicate"))
        {
            DublicateNode();
        }
        if (ImGui::Selectable("Disconnect"))
        {
            DisconnectAll();
        }
    }

    void GraphNode::DisconnectAll()
    {
        auto g = GetGraph();
        for (auto& pin : Inputs) {
            pin->DisconnectAll();
        }
        for (auto& pin : Outputs) {
            pin->DisconnectAll();
        }
    }

    void GraphNode::DeleteNode()
    {
        GetGraph()->RemoveNode(NodeId);
    }

    void GraphNode::DublicateNode()
    {
        ArcWarning("node dublications still in development")
    }

    bool GraphNode::OnMouseClick(WidgetEvent& e)
    {
        // left click: start node drag if clicked in header area
        if (e.Key == ImGuiMouseButton_Left)
        {
            auto gv = GetGraph();
            const auto& r = AbsoluteRect;
            // header region heuristic: top 22 px
            if (e.MousePos.y >= r.Min.y && e.MousePos.y <= r.Min.y + 22.0f)
            {
              //start drag
                gv->BeginDragNode(shared_from_this());
                return true;
            }

            return true;
        }
        return false;
    }

    bool GraphNode::OnMouseRelease(WidgetEvent& e)
    {
        return false;
    }

    bool GraphNode::OnMouseMove(WidgetEvent& e)
    {
        return false;
    }
    bool GraphNode::OnFocusGained(WidgetEvent& Even)
    {
        auto g = GetGraph();
        if (!g)
        {
            return false;
        }
        g->SetFocusedNode(shared_from_this());
        return true;
    }
    

}
