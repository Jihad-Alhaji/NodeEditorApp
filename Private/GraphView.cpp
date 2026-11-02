#include "GraphView.h"
#include"ArcApp.h"
#include<ArcRx/ImageManager.h>
#include<ArcLog/LogSystem.h>
#include<Hash.h>
#include<chrono>
#include <algorithm>
#include <tuple>
#include <charconv>
#include <cmath>
#include <set>
#include "NodeEditorRegistery.h"
#include "BlueprintGraph.h"
namespace NodeEditor
{

    GraphView::DocumentArchive GraphView::SaveDocument(const std::string& graphType) const
    {
        const auto& typeKey = DocumentGraphType.empty() ? graphType : DocumentGraphType;
        auto nodes = GetNodes();
        std::sort(nodes.begin(), nodes.end(), [](const auto& a, const auto& b) { return a->DocumentId < b->DocumentId; });
        nlohmann::json doc = {{"schema_version",1},{"graph_type",typeKey},{"next_node_id",std::to_string(NextDocumentNodeId)},{"nodes",nlohmann::json::array()},{"connections",nlohmann::json::array()}};
        for (const auto& node : nodes)
        {
            const auto type = NodeEditorRegistery::GetChecked().GetNodeTypeKey(typeKey,node.get());
            if (type.empty() || !std::isfinite(node->GraphPos.x) || !std::isfinite(node->GraphPos.y))
                throw std::runtime_error("Cannot save node with an unregistered type or non-finite position.");
            std::set<std::string> pinKeys;
            for (const auto* pins : { &node->Inputs, &node->Outputs }) for (const auto& pin : *pins)
                if (!pin || pin->PinKey.empty() || !pinKeys.insert(pin->PinKey).second)
                    throw std::runtime_error("Cannot save node with missing or duplicate PinKey metadata.");
            doc["nodes"].push_back({{"id",std::to_string(node->DocumentId)},{"type",type},{"position",{{"x",node->GraphPos.x},{"y",node->GraphPos.y}}},{"properties",node->SaveProperties()}});
        }
        struct Link { uint64_t a,b; std::string x,y; };
        std::vector<Link> links;
        for (const auto& c : GetConnections())
        {
            auto f=c->From.lock(), t=c->To.lock(); auto fn=f?f->ParentNode.lock():nullptr, tn=t?t->ParentNode.lock():nullptr;
            if (f&&t&&fn&&tn) links.push_back({fn->DocumentId,tn->DocumentId,f->PinKey,t->PinKey});
        }
        std::sort(links.begin(),links.end(),[](const auto& a,const auto& b){return std::tie(a.a,a.x,a.b,a.y)<std::tie(b.a,b.x,b.b,b.y);});
        for (const auto& l:links) doc["connections"].push_back({{"from",{{"node_id",std::to_string(l.a)},{"pin_key",l.x}}},{"to",{{"node_id",std::to_string(l.b)},{"pin_key",l.y}}}});
        return doc;
    }

    bool GraphView::LoadDocument(const DocumentArchive& doc,const std::string& graphType,std::string& diagnostic)
    {
        const auto& typeKey = DocumentGraphType.empty() ? graphType : DocumentGraphType;
        auto fail=[&](const std::string& s){diagnostic=s;return false;};
        try {
            if(!doc.is_object()) return fail("$: expected object");
            for(const char* k:{"schema_version","graph_type","next_node_id","nodes","connections"}) if(!doc.contains(k)) return fail(std::string("$.")+k+": required field missing");
            if(!doc["schema_version"].is_number_integer()||doc["schema_version"].get<int>()!=1) return fail("$.schema_version: unsupported version (expected 1)");
            if(!doc["graph_type"].is_string()||doc["graph_type"]!=typeKey) return fail("$.graph_type: does not match active graph");
            auto& registry=NodeEditorRegistery::GetChecked(); if(!registry.GetGraphFactory(typeKey)) return fail("$.graph_type: unregistered graph type");
            auto parse=[](const nlohmann::json& j,uint64_t& n){if(!j.is_string())return false;auto s=j.get<std::string>();if(s.empty()||s[0]=='0'||!std::all_of(s.begin(),s.end(),[](char c){return c>='0'&&c<='9';}))return false;auto r=std::from_chars(s.data(),s.data()+s.size(),n);return r.ec==std::errc{}&&r.ptr==s.data()+s.size()&&n>0;};
            uint64_t next=0;if(!parse(doc["next_node_id"],next))return fail("$.next_node_id: expected canonical positive decimal string");
            if(!doc["nodes"].is_array()||!doc["connections"].is_array())return fail("$.nodes and $.connections must be arrays");
            std::unordered_map<uint64_t,std::shared_ptr<GraphNode>> byId; std::vector<std::shared_ptr<GraphNode>> staged; uint64_t maxId=0;
            auto types=registry.GetRegisteredNodeTypes(typeKey);
            for(size_t i=0;i<doc["nodes"].size();++i){const auto& j=doc["nodes"][i];auto p="$.nodes["+std::to_string(i)+"]";if(!j.is_object())return fail(p+": expected object");for(const char*k:{"id","type","position","properties"})if(!j.contains(k))return fail(p+"."+k+": required field missing");uint64_t id=0;if(!parse(j["id"],id))return fail(p+".id: invalid positive decimal string");if(byId.count(id))return fail(p+".id: duplicate ID");if(!j["type"].is_string())return fail(p+".type: expected string");auto type=j["type"].get<std::string>();if(std::find(types.begin(),types.end(),type)==types.end())return fail(p+".type: unregistered node type '"+type+"'");auto n=registry.SpawnNode(typeKey,type);if(!n)return fail(p+".type: factory failed");if(!j["position"].is_object()||!j["position"].contains("x")||!j["position"].contains("y")||!j["position"]["x"].is_number()||!j["position"]["y"].is_number())return fail(p+".position: expected numeric x and y");float x=j["position"]["x"].get<float>(),y=j["position"]["y"].get<float>();if(!std::isfinite(x)||!std::isfinite(y))return fail(p+".position: coordinates must be finite");std::string error;if(!n->LoadProperties(j["properties"],error))return fail(p+".properties: "+error);n->DocumentId=id;n->TypeKey=type;n->GraphPos={x,y};std::set<std::string> keys;for(auto* pins:{&n->Inputs,&n->Outputs})for(auto& pin:*pins)if(!pin||pin->PinKey.empty()||!keys.insert(pin->PinKey).second)return fail(p+": factory PinKeys must be present and unique");byId[id]=n;staged.push_back(n);maxId=std::max(maxId,id);}
            if(next<=maxId)return fail("$.next_node_id: must exceed all node IDs");
            struct StagedLink { uint64_t fromId,toId; std::string fromKey,toKey; };
            std::vector<StagedLink> linkSpecs;std::set<std::tuple<uint64_t,std::string,uint64_t,std::string>> unique;std::unordered_map<GraphPin*,size_t> counts;
            for(size_t i=0;i<doc["connections"].size();++i){const auto& j=doc["connections"][i];auto p="$.connections["+std::to_string(i)+"]";if(!j.is_object()||!j.contains("from")||!j.contains("to"))return fail(p+": requires from and to");auto getEnd=[&](const nlohmann::json&e,const char* side,const char* dir,uint64_t& id,std::shared_ptr<GraphNode>&owner,std::shared_ptr<GraphPin>&pin)->std::string{if(!e.is_object()||!e.contains("node_id")||!e.contains("pin_key"))return std::string(".")+side+": malformed endpoint (requires node_id and pin_key)";if(!parse(e["node_id"],id))return std::string(".")+side+".node_id: invalid ID";auto it=byId.find(id);if(it==byId.end())return std::string(".")+side+".node_id: missing node";if(!e["pin_key"].is_string())return std::string(".")+side+".pin_key: expected string";if(e.contains("direction")&&(!e["direction"].is_string()||e["direction"]!=dir))return std::string(".")+side+".direction: expected legacy string '"+dir+"'";owner=it->second;auto& ps=std::string(dir)=="input"?owner->Inputs:owner->Outputs;auto f=std::find_if(ps.begin(),ps.end(),[&](const auto& q){return q&&q->PinKey==e["pin_key"].get<std::string>();});if(f==ps.end())return std::string(".")+side+".pin_key: unknown pin for endpoint slot '"+dir+"'";pin=*f;return {};};uint64_t aid=0,bid=0;std::shared_ptr<GraphNode>a,b;std::shared_ptr<GraphPin>f,t;auto e=getEnd(j["from"],"from","output",aid,a,f);if(!e.empty())return fail(p+e);e=getEnd(j["to"],"to","input",bid,b,t);if(!e.empty())return fail(p+e);if(a==b)return fail(p+": self link");if(!f->AllowConnectionWith(t)||!t->AllowConnectionWith(f))return fail(p+": pin compatibility rejected link");auto key=std::make_tuple(aid,f->PinKey,bid,t->PinKey);if(!unique.insert(key).second)return fail(p+": duplicate link");if((!f->AllowMultipleConnections()&&counts[f.get()]++)||(!t->AllowMultipleConnections()&&counts[t.get()]++))return fail(p+": pin cardinality rejected link");linkSpecs.push_back({aid,bid,f->PinKey,t->PinKey});}
            ReplaceContents(std::move(staged),{},next);
            for(const auto& spec:linkSpecs){auto findPin=[&](uint64_t id,const std::string& key,bool output){auto node=std::find_if(Nodes.begin(),Nodes.end(),[&](const auto& e){return e.second->DocumentId==id;});if(node==Nodes.end())return std::shared_ptr<GraphPin>{};auto& pins=output?node->second->Outputs:node->second->Inputs;auto pin=std::find_if(pins.begin(),pins.end(),[&](const auto& p){return p&&p->PinKey==key;});return pin==pins.end()?std::shared_ptr<GraphPin>{}:*pin;};auto from=findPin(spec.fromId,spec.fromKey,true),to=findPin(spec.toId,spec.toKey,false);auto connection=std::make_shared<GraphConnection>(from,to);if(Connections.count(connection->ConnectionId))return fail("$.connections: generated connection ID collision");connection->Connect();Connections[connection->ConnectionId]=connection;}
            DocumentGraphType = typeKey;
            diagnostic.clear();return true;
        }catch(const std::exception&e){return fail(std::string("$: invalid document: ")+e.what());}
    }

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

    bool GraphView::Load(const DocumentArchive& archive)
    {
        if (DocumentGraphType.empty()) { PersistenceDiagnostic = "Graph type is not registered with a stable factory key."; return false; }
        return LoadDocument(archive, DocumentGraphType, PersistenceDiagnostic);
    }

    void GraphView::Save(DocumentArchive& archive)
    {
        if (DocumentGraphType.empty()) { PersistenceDiagnostic = "Graph type is not registered with a stable factory key."; return; }
        try { archive = SaveDocument(DocumentGraphType); PersistenceDiagnostic.clear(); }
        catch (const std::exception& e) { PersistenceDiagnostic = e.what(); }
    }

    void GraphView::AddNode(std::shared_ptr<GraphNode> node)
    {
        if (!node)
        {
            return;
        }

        node->NodeId = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count();
        hash_combine(node->NodeId, ++NodeIdCounter);
        if (node->DocumentId == 0) node->DocumentId = NextDocumentNodeId++;
        else NextDocumentNodeId = std::max(NextDocumentNodeId, node->DocumentId + 1);

        Nodes[node->NodeId] = node;
        AddChild(node); 
    }

    std::vector<std::shared_ptr<GraphNode>> GraphView::GetNodes() const
    {
        std::vector<std::shared_ptr<GraphNode>> result;
        for (const auto& entry : Nodes) result.push_back(entry.second);
        return result;
    }

    std::vector<std::shared_ptr<GraphConnection>> GraphView::GetConnections() const
    {
        std::vector<std::shared_ptr<GraphConnection>> result;
        for (const auto& entry : Connections) result.push_back(entry.second);
        return result;
    }

    void GraphView::ReplaceContents(std::vector<std::shared_ptr<GraphNode>> nodes, std::vector<std::shared_ptr<GraphConnection>> connections, uint64_t nextId)
    {
        while (!Connections.empty()) RemoveConnection(Connections.begin()->first);
        Connections.clear();
        Nodes.clear();
        Children.clear();
        NodeIdCounter = 0;
        NextDocumentNodeId = 1;
        for (auto& node : nodes) AddNode(std::move(node));
        for (auto& connection : connections)
        {
            if (!connection) continue;
            auto from = connection->From.lock(); auto to = connection->To.lock();
            if (!from || !to) continue;
            connection = std::make_shared<GraphConnection>(from, to);
            connection->Connect();
            Connections[connection->ConnectionId] = connection;
        }
        NextDocumentNodeId = nextId;
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
        // basic connection rules
		auto f = conn->From.lock();
		auto t = conn->To.lock();

		//check if pins are valid and not the same pin
		if (!f || !t)
		{
			return;
		}
		
		//prevent connecting pins on the same node
		if (f->ParentNode.lock() == t->ParentNode.lock())
		{
			return;
		}

		//respect pin connection rules
        if (!f->AllowMultipleConnections() && f->IsConnected())
        {
            return;
        }

		if (!t->AllowMultipleConnections() && t->IsConnected())
		{
			return;
		}

		//respect pin connection rules
		if (!f->AllowConnectionWith(t) || !t->AllowConnectionWith(f))
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
