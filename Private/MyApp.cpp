
#include "MyApp.h"
#include<imgui/imgui.h>
#include"Widget.h"
#include"UIManager.h"
#include<ArcLog/LogSystem.h>
#include"NGEditor.h"
#include"NodeEditorRegistery.h"
#include"BlueprintGraph.h"
#include"BlueprintPins.h"


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
	ImFont* myFont = fontAtlas->AddFontFromFileTTF((GetResourcesDir() + "Fonts/Lato-Regular.ttf").c_str(), 20);
	io.FontDefault = myFont;

	
	m_CloseTex = Graphics::ResourceTexManager::GetOrCreate(GetResourcesDir() + "Icons/close.png");
	m_MaximizeTex = Graphics::ResourceTexManager::GetOrCreate(GetResourcesDir() + "Icons/fullscreen.png");
	m_MinimizeTex = Graphics::ResourceTexManager::GetOrCreate(GetResourcesDir() + "Icons/minimize.png");

	//initialize node editor registary
	NodeEditorRegistery::Initialize();

	Lua().script_file(GetResourcesDir() + "Scripts/main.lua");

    mUIRoot = std::make_shared<UIManager>(Vec2( m_WinWidth, m_WinHeight ));
	

	//initialize blueprint graphs
	RegisterBlueprintGraph();

	//create a blueprint graph editor
	{
		auto g = std::make_shared<NodeEditor::NGEditor>(NodeEditorRegistery::Get()->SpawnGraph("Blueprint"));
		g->SetAlignment(EAlign::Center, EAlign::Center);
		g->SetSize({ 1080,720 });
		mUIRoot->AddRootWidget(g);


		OnAppSave.Bind(1001,[this, g](Serialzer_t& ar) {

			ResourceManagement::Serialization::SaveToFile(*g->GetEditedGraph(), GetResourcesDir() + "BlueprintGraph.json");
		});
		OnAppLoad.Bind(1001, [this, g](const Serialzer_t& ar) {
			ResourceManagement::Serialization::LoadFromFile(*g->GetEditedGraph(), GetResourcesDir() + "BlueprintGraph.json");
		});
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
	mUIRoot->HandleEvents();
	mUIRoot->Draw();
}

void MyApp::Tick(double dt)
{
	ArcApp::Tick(dt);
	mUIRoot->Tick(dt);
	
}
