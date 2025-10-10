#include "GraphNode.h"
#include "GraphView.h" 
namespace NodeEditor
{

    GraphNode::GraphNode(const std::string& title, const ImVec2& graphPos)
        : Widget("GraphNode"), Title(title), GraphPos(graphPos)
    {
        Size = NodeSize;
        bTickAllowed = false;
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

    // add new pin and return shared_ptr
    std::shared_ptr<GraphPin> GraphNode::AddPin(EPinType type, const std::string& name, ImVec2 localPos)
    {
        auto p = std::make_shared<GraphPin>(type, name, localPos);
        p->ParentNode = shared_from_this();
        if (type == EPinType::Input) Inputs.push_back(p); else Outputs.push_back(p);
        return p;
    }

    Rect GraphNode::GetRect() const
    {
        // AbsoluteRect is set by UpdateLayout; use it
        return AbsoluteRect;
    }

    GraphView* GraphNode::GetGraph() const
    {
        return dynamic_cast<GraphView*>(Parent);
    };

    // Drawing uses AbsoluteRect (screen)
    void GraphNode::Draw()
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 a = AbsoluteRect.Min;
        ImVec2 b = AbsoluteRect.Max;

        // Node background
        dl->AddRectFilled(a, b, IM_COL32(60, 60, 60, 220), 6.0f);
        
        // Header bar
        ImVec2 headerB = ImVec2(b.x, a.y + 22.0f);
        dl->AddRectFilled(a, headerB, IM_COL32(30, 30, 30, 220), 6.0f);

        // Title
        dl->AddText(ImVec2(a.x + 8.0f, a.y + 4.0f), IM_COL32_WHITE, Title.c_str());

        // Body text (placeholder)
        dl->AddText(ImVec2(a.x + 8.0f, a.y + 30.0f), IM_COL32(200, 200, 200, 255), "Node content");

        // Draw pins (circles)
        auto drawPin = [&](const std::shared_ptr<GraphPin>& pin, ImU32 color)
        {
            ImVec2 p = pin->GetScreenPos(static_cast<GraphView*>(GetParent()));
            dl->AddCircleFilled(p, 6.0f, color);
        };

        for (auto& pin : Inputs)
            drawPin(pin, IM_COL32(180, 180, 180, 255));
        for (auto& pin : Outputs)
            drawPin(pin, IM_COL32(180, 180, 180, 255));

        if (bFocused)
        {
            dl->AddRect(a, b, IM_COL32(80, 120, 180, 220), 6.f, 0, 3.f);
        }
    }

    inline float ImLengthSqr(const ImVec2& a, const ImVec2& b)
    {
        ImVec2 temp = a - b;
        temp *= temp;
        return temp.x + temp.y;
    }

    bool GraphNode::OnMouseClick(WidgetEvent& e)
    {
        // left click: start node drag if clicked in header area
        if (e.Key == ImGuiMouseButton_Left)
        {
            auto gv = GetGraph();
            auto r = GetRect();
            // header region heuristic: top 22 px
            if (e.MousePos.y >= r.Min.y && e.MousePos.y <= r.Min.y + 22.0f)
            {
              //start drag
                gv->BeginDragNode(shared_from_this());
                return true;
            }

            return true;
        }

        // right click could be context menu - ignore here
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

    // GraphPin screen position implementation
    ImVec2 GraphPin::GetScreenPos(const GraphView* view) const
    {
        auto nodePtr = ParentNode.lock();
        if (!nodePtr || !view) return ImVec2(0, 0);
        // node absolute rect
        Rect NRect = nodePtr->GetAbsoluteRect();
        ImVec2 offset = NRect.Min;
        if (Type == EPinType::Output)
        {
            offset.x = NRect.Max.x;
        }
        return offset + LocalPos;
    }
}