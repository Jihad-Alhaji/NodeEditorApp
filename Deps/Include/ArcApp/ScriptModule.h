#pragma once
#include<IAppModule.h>
#include"ScriptableBase.h"

//a module that can be ingerited in lua
class ScriptModule : public ARC::App::IAppModuleInterface, public ScriptableBase
{

public:
	bool Load(const Serialzer_t& Archive);
	void Save(Serialzer_t& Archive);
	void Unload();

	void OnStart();
	void OnEnd();

	void Tick(double DeltaTime);
	void Draw(double DeltaTime);
	void BuildMenus();

	//gets the unique module name
	std::string GetModuleName()const override { return Name; };
	std::string Name = "Script Module";
};

