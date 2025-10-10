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
        GraphView* view = dynamic_cast<GraphView*>(Parent);
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

    // Drawing uses AbsoluteRect (screen)
    void GraphNode::Draw()
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 a = AbsoluteRect.Min;
        ImVec2 b = AbsoluteRect.Max;

        // Node background
        ImU32 bg = bSelected ? IM_COL32(80, 120, 180, 220) : IM_COL32(60, 60, 60, 220);
        dl->AddRectFilled(a, b, bg, 6.0f);

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
            auto r = GetRect();
            if (r.Contains(e.MousePos))
            {
                // header region heuristic: top 22 px
                if (e.MousePos.y >= r.Min.y && e.MousePos.y <= r.Min.y + 22.0f)
                {
                    bDragging = true;
                    DragOffset = e.MousePos - AbsoluteRect.Min;
                    bSelected = true;
                    return true;
                }

                // check pin clicks: if a pin hit -> start link via parent GraphView
                // check Inputs
                for (auto& pin : Inputs)
                {
                    ImVec2 pinPos = pin->GetScreenPos(static_cast<GraphView*>(GetParent()));
                    float dist = ImLengthSqr(pinPos, e.MousePos);
                    if (dist <= (8.0f * 8.0f))
                    {
                        // request parent GraphView to start link from this pin
                        if (auto gv = dynamic_cast<GraphView*>(GetParent()))
                        {
                            gv->BeginLinkFromPin(pin);
                            return true;
                        }
                    }
                }
                // check Outputs
                for (auto& pin : Outputs)
                {
                    ImVec2 pinPos = pin->GetScreenPos(static_cast<GraphView*>(GetParent()));
                    float dist = ImLengthSqr(pinPos, e.MousePos);
                    if (dist <= (8.0f * 8.0f))
                    {
                        if (auto gv = dynamic_cast<GraphView*>(GetParent()))
                        {
                            gv->BeginLinkFromPin(pin);
                            return true;
                        }
                    }
                }

                // clicked inside node body (not header): select but do not start drag
                bSelected = true;
                return true;
            }
        }

        // right click could be context menu - ignore here
        return false;
    }

    bool GraphNode::OnMouseRelease(WidgetEvent& e)
    {
        if (e.Key == ImGuiMouseButton_Left)
        {
            if (bDragging)
            {
                bDragging = false;
                return true;
            }
        }
        return false;
    }

    bool GraphNode::OnMouseMove(WidgetEvent& e)
    {
        if (bDragging)
        {
            // convert mouse position minus drag offset into graph coords using parent GraphView
            if (auto gv = dynamic_cast<GraphView*>(GetParent()))
            {
                ImVec2 screenTopLeft = e.MousePos - DragOffset;
                ImVec2 newGraphPos = gv->ScreenToGraph(screenTopLeft);
                GraphPos = newGraphPos;
                return true;
            }
        }
        return false;
    }

    // GraphPin screen position implementation
    ImVec2 GraphPin::GetScreenPos(const GraphView* view) const
    {
        auto nodePtr = ParentNode.lock();
        if (!nodePtr || !view) return ImVec2(0, 0);
        // node absolute rect
        ImVec2 nodeMin = nodePtr->GetAbsoluteRect().Min;
        // local pos in node coords, but node's content may scale or offset; we assume LocalPos is in node space
        return ImVec2(nodeMin.x + LocalPos.x, nodeMin.y + LocalPos.y);
    }
}