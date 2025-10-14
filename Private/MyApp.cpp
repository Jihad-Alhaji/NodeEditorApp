
#include "MyApp.h"
#include<imgui/imgui.h>
#include"Widget.h"
#include"UIManager.h"
#include<ArcLog/LogSystem.h>
#include"NGEditor.h"
#include"NodeEditorRegistery.h"
#include"BlueprintGraph.h"


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
	
	NodeEditorRegistery::Initialize();
	//Debug test registerations
	RegisterBlueprintGraph();
	{
		
		auto g = std::make_shared<NodeEditor::NGEditor>(NodeEditorRegistery::Get()->SpawnGraph("Blueprint"));
		g->SetAlignment(EAlign::Center, EAlign::Center);
		g->SetSize({ 1080,720 });
		mUIRoot->AddRootWidget(g);
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
