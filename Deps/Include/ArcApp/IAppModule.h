#pragma once
#include"ResourceManagement/ISerializationInterface.h"
namespace ARC::App
{
	//interface for app extensions
	class IAppModuleInterface : public ResourceManagement::Serialization::ISerializationInterface
	{

	public:
		//serialization interface
		//virtual bool Load(const Serialzer_t& Archive);
		//virtual void Save(Serialzer_t& Archive);
		//virtual void Unload();

		virtual void OnStart() {};
		virtual void OnEnd() {};

		virtual void Tick(double DeltaTime) {};
		virtual void Draw(double DeltaTime) {};
		virtual void BuildMenus() {};

		//gets the unique module name
		virtual std::string GetModuleName()const = 0;

		//utility func for getting the app instance
		static class ArcApp* GetAppInstance();
	};
}


