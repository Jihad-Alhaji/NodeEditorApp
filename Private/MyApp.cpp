
#include "MyApp.h"
#include<imgui/imgui.h>
#include"Widget.h"
#include"UIManager.h"
#include<ArcLog/LogSystem.h>
#include"GraphView.h"

bool MyApp::Init()
{
	if (!ArcApp::Init())
	{
		return false;
	}

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	
	// Load and set up fonts
	ImFontAtlas* fontAtlas = io.Fonts;
	fontAtlas->Clear();
	ImFont* myFont = fontAtlas->AddFontFromFileTTF((GetResourcesDir() + "Fonts/Lato-Regular.ttf").c_str(), 24);
	io.FontDefault = myFont;

	Lua().script_file(GetResourcesDir() + "Scripts/main.lua");

    mUIRoot = std::make_shared<UIManager>(Vec2( m_WinWidth, m_WinHeight ));

	{
		auto g = std::make_shared<NodeEditor::GraphView>();
		g->SetAlignment(EAlign::Center, EAlign::Center);
		mUIRoot->AddRootWidget(g);
		auto n = std::make_shared<NodeEditor::GraphNode>("test 1", ImVec2{ 0,0 });
		n->AddPin(NodeEditor::EPinType::Input, "in", {});
		n->AddPin(NodeEditor::EPinType::Output, "out", {});
		g->AddNode(n);

		auto b = std::make_shared<NodeEditor::GraphNode>("test 1", ImVec2{ 100,0 });
		b->AddPin(NodeEditor::EPinType::Input, "in", {});
		b->AddPin(NodeEditor::EPinType::Output, "out", {});
		g->AddNode(b);
	}
	return true;
}

void MyApp::Draw(double dt)
{
    ImGui::NewLine();

    auto p = ImGui::GetWindowPos();
    auto s = ImGui::GetWindowSize() + p;
    mUIRoot->SetViewRect({ p, s });
    mUIRoot->UpdateLayout();

	mUIRoot->Draw();
}

void MyApp::Tick(double dt)
{
	ArcApp::Tick(dt);
	mUIRoot->Tick(dt);
	mUIRoot->HandleEvents();
}
