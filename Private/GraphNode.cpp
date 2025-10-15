#include "GraphNode.h"
#include "GraphView.h" 
#include<ArcLog/LogSystem.h>
namespace NodeEditor
{
    GraphPin::GraphPin() :Widget("pin")
    {
        bTickAllowed = false;
        Size = { 16,16 };
        HorizontalAlign = EAlign::Center;
        VerticalAlign = EAlign::Top;
        bDraggable = true;
    }
    void GraphPin::Draw()
    {
        ImGui::PushID(this);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 center = AbsoluteRect.Center();
        float  radius = 8;
        ImGui::SetCursorScreenPos(AbsoluteRect.Min);
        ImGui::InvisibleButton("btn", AbsoluteRect.Size());//hit box 
        if (ImGui::BeginPopupContextItem("PinCtxMenu"))
        {
            DrawContextMenu();
            ImGui::EndPopup();
        }
        dl->AddCircleFilled(center, radius, IM_COL32(20, 220, 210, 220));
        if (bHovered)
        {
            dl->AddCircle(center, radius + 1, IM_COL32(220, 200, 20, 220),0.f,2.f);
        }

        if (bDragged)
        {
            ImDrawList* dl = ImGui::GetForegroundDrawList();
            ImVec2 mPos = ImGui::GetMousePos();
            float offset = Type == EPinType::Output ? 50.f : -50.f;
           
            dl->AddBezierCubic(center, {center.x + offset,center.y}, { mPos.x - offset,mPos.y }, mPos, IM_COL32(20, 220, 210, 220), 5.f);
        }
        ImGui::PopID();
    }
   
    void GraphPin::DrawContextMenu()
    {
        ImGui::Text("pin ctx menu");
    }

    bool GraphPin::OnRecieveDrop(WidgetEvent& e)
    {
        GraphPin* inPin = static_cast<GraphPin*>(e.Payload);
        //connect pins of different types, that are not the same pin and that are on different nodes
        if (inPin && inPin != this && inPin->Type != Type && ParentNode.lock() != inPin->ParentNode.lock())
        {
            GraphView* g = ParentNode.lock()->GetGraph();
            //always make connects from an output pin to an input pin
            if (Type == EPinType::Input)
            {
                g->AddConnection({ inPin->shared_from_this() , shared_from_this() });
            }
            else
            {
                g->AddConnection({ shared_from_this(),inPin->shared_from_this() });
            }
           
            return true;
        }
        return false;
    }

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
            Inputs.emplace_back(std::move(pin));
        }
        else
        {
            VB_OutPins->AddChild(pin);
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

        // Body text (placeholder)
        dl->AddText(ImVec2(a.x + 8.0f, a.y + 30.0f), IM_COL32(200, 200, 200, 220), "Node content");
        
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
        ImGui::Text("node ctx menu");
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