#include "GraphPin.h"
#include "GraphNode.h"
#include "GraphView.h"
#include <Hash.h>
namespace NodeEditor
{
    GraphConnection::GraphConnection(std::shared_ptr<GraphPin> from, std::shared_ptr<GraphPin> to) :
        From{ from }, To{ to }
    {
        ConnectionId = from->ParentNode.lock()->NodeId;
        hash_combine(ConnectionId, from->PinId, to->ParentNode.lock()->NodeId, to->PinId);
    }
  
    void GraphConnection::Connect()
    {
        auto f = From.lock();
        auto t = To.lock();
        if (f && t)
        {
            auto ref = shared_from_this();
            f->Connections[ConnectionId] = ref;
            t->Connections[ConnectionId] = ref;
        }
    }

    void GraphConnection::Disconnect()
    {
        auto f = From.lock();
        auto t = To.lock();
        if (f && t)
        {
            f->Connections.erase(ConnectionId);
            t->Connections.erase(ConnectionId);
        }
    }

    GraphPin::GraphPin() :Widget("pin")
    {
        bTickAllowed = false;
        Size = { 16,16 };
        HorizontalAlign = EAlign::Center;
        VerticalAlign = EAlign::Top;
        bDraggable = true;
    }
    bool GraphPin::AllowMultipleConnections() const
    {
		//flow pins can have one out connection, and multiple in connections, data pins can have multiple connections
		if (IsFlowPin())
		{
			return Type == EPinType::Output? false : true;
		}
        return true;
    }

    bool GraphPin::AllowConnectionWith(std::shared_ptr<GraphPin> other) const
    {
        if (!other)
        {
            return false;
        }
        //flow pins can only connect to flow pins, data pins can only connect to data pins
        if (Role != other->Role)
        {
            return false;
        }
        return true;
    }

    void GraphPin::Draw()
    {
        ImGui::PushID(this);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 center = AbsoluteRect.Center();
        float  radius = 8;
        auto parent = ParentNode.lock();
        auto graph = parent ? parent->GetGraph() : nullptr;
        float zoom = graph ? graph->GetZoom() : 1.f;
        ImGui::SetCursorScreenPos(AbsoluteRect.Min);
        ImGui::InvisibleButton("btn", AbsoluteRect.Size());//hit box 
        if (ImGui::BeginPopupContextItem("PinCtxMenu"))
        {
            DrawContextMenu();
            ImGui::EndPopup();
        }
        dl->AddCircleFilled(center, radius, IM_COL32(20, 220, 210, 220));
        const char* pinLabel = Name.c_str();
        float labelWidth = ImGui::CalcTextSize(pinLabel).x;
        ImVec2 textPos = Type == EPinType::Input
            ? ImVec2(center.x + radius + 4.f, center.y - ImGui::GetTextLineHeight() * 0.5f)
            : ImVec2(center.x - radius - 4.f - labelWidth, center.y - ImGui::GetTextLineHeight() * 0.5f);
        dl->AddText(textPos, IM_COL32(220, 220, 220, 230), Name.c_str());
        if (bHovered)
        {
            dl->AddCircle(center, radius + 1, IM_COL32(220, 200, 20, 220),0.f,2.f);
        }

        if (bDragged)
        {
            ImDrawList* dl = ImGui::GetForegroundDrawList();
            ImVec2 mPos = ImGui::GetMousePos();
            float offset = (Type == EPinType::Output ? 50.f : -50.f) * zoom;
           
            dl->AddBezierCubic(center, {center.x + offset,center.y}, { mPos.x - offset,mPos.y }, mPos, IM_COL32(20, 220, 210, 220), 5.f);
        }
        ImGui::PopID();
    }
   
    void GraphPin::DrawContextMenu()
    {
        if (ImGui::Selectable("Disconnect"))
        {
            DisconnectAll();
        }
    }

    bool GraphPin::OnRecieveDrop(WidgetEvent& e)
    {
        GraphPin* inPin = static_cast<GraphPin*>(e.Payload);
        //connect pins of different types, that are not the same pin and that are on different nodes
        if (inPin && inPin != this && inPin->Type != Type && ParentNode.lock() != inPin->ParentNode.lock())
        {
            GraphView* g = ParentNode.lock()->GetGraph();

            //always make connects from an output pin to an input pin
            std::shared_ptr<GraphConnection> conn{};
            if (Type == EPinType::Input)
            {
                conn = std::make_shared<GraphConnection>(inPin->shared_from_this(), shared_from_this());
            }
            else
            {
                conn = std::make_shared<GraphConnection>(shared_from_this() , inPin->shared_from_this());
            }

            //add connection to graph
            g->AddConnection(std::move(conn));
            return true;
        }
        return false;
    }

    void GraphPin::DisconnectAll()
    {
        if (!IsConnected())
        {
            return;
        }
        auto g = ParentNode.lock()->GetGraph();
        //this safely loops until all connections has be removed
        auto oldConns = std::move(Connections);
        for (const auto& [id,conn] : oldConns) {
            g->RemoveConnection(id);
        }
    }


}

