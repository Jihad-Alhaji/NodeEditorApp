#pragma once
#include"AppComponents/IServiceProvider.h"
#include"ResourceManagement/ISerializationInterface.h"
#include"Structures/Delegate.h"
#include"Window.h"
#include<memory>
#include"Event.h"
#include"AppComponents/TaskManager.h"
#include<set>
#include"sol/sol.hpp"
#include"PluginManager.h"

namespace Graphics
{
	class SimpleTexture;
}

namespace ARC::App
{
	class ArcApp : public AppComponents::IServiceProvider, protected ResourceManagement::Serialization::ISerializationInterface
	{
	public:
		ArcApp();

		//gets a global  app pointer
		static ArcApp* GetInstance();

		virtual ~ArcApp();

		//initializes the app: loads config, creates window, initializes services
		virtual bool Init();

		//starts the main app loop
		virtual void Start();

		//called every tick, use it to handle app logic
		virtual void Tick(double DeltaTime);
		virtual void Draw(double DeltaTime);
		
		//stops the app and frees up any loaded resources, NOTE: call requeststop 
		virtual void Stop();
		void RequestStop();

		void AttachModule(class IAppModuleInterface* appModule);
		void DetachModule(class IAppModuleInterface* appModule);
		class IAppModuleInterface* GetModule(std::string_view modulename)const;
	
		//loads the app data
		void LoadAppData();

		//saves the app data
		void SaveAppData();

		//loads the app config
		bool LoadConfig();

		//saves the app config
		void SaveConfig();

		//sets the delay between ticks
		void SetUpdateDelay(double delay);
		double GetDeltaTime()const { return m_DeltaTime; }

		//app resources folder path
		virtual std::string GetResourcesDir()const { return "Resources/"; };
		virtual std::string GetConfigFilePath()const;
		virtual std::string GetDataFilePath()const;

		//event management

		//add an event to be executed by the app, can be delayed too, true if it was submitted correctly
		bool PushEvent(ARC::Event e);

		void AddCommand(std::string cmd, Event&& e);
		void RemoveCommand(std::string cmd);
		void ExecuteCommand(std::string cmd);

		void SetAppTheme(std::string ThemeName, bool force = false);
		void RegisterTheme(std::string ThemeName, Event&& e);

		inline PluginManager& GetPluginManager(){ return m_PluginManager; }
		inline const PluginManager& GetPluginManager()const { return m_PluginManager; }

		std::shared_ptr<AppComponents::TaskManager> GetTaskManager();
		
		inline sol::state& Lua() { return m_lua; }
		inline const sol::state& Lua() const { return m_lua; }

		//event for app tick, called every tick
		Structures::TMultiDelegate<double> OnAppTick;

		//called to draw external ui, called after the app draws itself
		Structures::TMultiDelegate<double> OnAppDraw;
		
		//extends menubar, after the app builds it
		Structures::TMultiDelegate<> OnAppBuildMenuBar;

		//on app startsloading, after the app loads
		Structures::TMultiDelegate< const Serialzer_t&> OnAppLoad;

		//on app starts saving after the app saves
		Structures::TMultiDelegate<Serialzer_t&> OnAppSave;

		//on app unloading, ideally before the  app unloads
		Structures::TMultiDelegate<> OnAppUnload;

		//when the app starts
		Structures::TMultiDelegate<> OnAppStart;

		//when the app closes
		Structures::TMultiDelegate<> OnAppStop;
	
	protected:
		//serialization interface, the save and load saves the app settings, config 
		bool Load(const Serialzer_t& Archive)override;
		//save to app confif
		void Save(Serialzer_t& Archive) override;
		void Unload() override;

		//load the app data
		virtual bool LoadAppData(const Serialzer_t& Archive);
		//save the app data
		virtual void SaveAppData(Serialzer_t& Archive);

		//used to  register with lua
		virtual void InitLua();

		double m_DeltaTime;

		std::unique_ptr<ARC::App::Window> m_Win;

		//settings
		double m_UpdateDelay;
		float m_ClearColor[4];
		std::string m_AppName;
		int m_WinWidth;
		int m_WinHeight;

		bool m_UsesViewports			:1 = true;//if true, the app will create seperate windows for child wins
		bool m_MenuBarVisible			:1 = true;
		bool m_TitleBarHovered			:1 = false;
		bool m_ShowThemeSelectionMenu	:1 = true;
		bool m_bRunning					:1	= false;
		
		std::shared_ptr<Graphics::SimpleTexture> m_CloseTex;
		std::shared_ptr<Graphics::SimpleTexture> m_MaximizeTex;
		std::shared_ptr<Graphics::SimpleTexture> m_MinimizeTex;

		//a theme command name
		std::string m_AppTheme;

		//used to build the menu bar
		virtual void BuildMenuBar();

		sol::state m_lua;

		PluginManager m_PluginManager;
	private:

		//creates the main window title bar
		void DrawTitleBar(float& barheight);
		void DrawMenuBar();

		//internal update function, this does the draw ans tick calls 
		void Update();

		//draws the core elements of the app and calls the virtual draw functions
		void Draw_Internal(double deltatime);

		EventDispatcher m_EventsDispatcher;
		std::map<std::string, IAppModuleInterface*> m_Modules;
		std::map<std::string, std::unique_ptr<Event>> m_Commands;

		std::set<std::string> m_ThemesList;
	};
}


