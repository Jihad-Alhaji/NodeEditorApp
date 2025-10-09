#pragma once
#include<string>
#include<map>
#include<ResourceManagement/Serializer.h>
#include"ResourceManagement/ISerializationInterface.h"


enum class EPluginLoadMode
{
	Immediate,
	OnDemand
};

struct FPluginDescription : public ResourceManagement::Serialization::ISerializationInterface
{
	std::string Name; // Name of the plugin
	std::string Version; // Version of the plugin
	std::string Author; // Author of the plugin
	std::string Description; // Description of the plugin
	std::vector<std::string> Dependencies; // List of dependencies for the plugin
	EPluginLoadMode LoadMode; // Load mode for the plugin

	bool Load(const Serialzer_t& Archive)override;
	
	void Save(Serialzer_t& Archive)override;
};

struct FPluginHandle
{
	void* Handle; // Handle to the plugin DLL
	class IPluginInterface* PluginPtr; // Pointer to the plugin interface
	FPluginDescription PluginDesc; // Description of the plugin

	inline bool IsLoaded()const { return PluginPtr != nullptr; };
};

namespace ARC
{
	namespace App {
		class ArcApp;
	}
}

class PluginManager
{
	//map available plugins
	std::map<std::string, FPluginHandle> PluginMap{};

	friend class ARC::App::ArcApp;

	PluginManager() = default;

	// Loads a specific plugin from the given path, internal use only: this does not respect dependencies
	bool LoadPlugin_internal(const std::string& desc, std::string_view Dir);

public:
	// Loads all plugins from the Plugins directory
	void DiscoverPlugins();

	//tries to load a plugin given its name, will also checks dependenceis and load then if needed
	bool LoadPlugin(const std::string& pluginName, bool loadDependecies = true);

	bool UnloadPlugin(const std::string& pluginName);

	std::string GetPluginsDir();

	//gets a pointer to the given plugin if it is loaded
	class IPluginInterface* GetPlugin(std::string Name);

	//is plugin available and loaded
	inline bool IsPluginLoaded(const std::string& PName)const;

	//is plugin available regardless of load status
	inline bool IsPluginAvailable(const std::string& PName)const;

	void GetAllAvailablePlugins(std::vector<FPluginDescription>& result);
};


namespace ResourceManagement::Serialization
{
	template<>
	void Serialize(Serialzer_t& Archive, const FPluginDescription& desc);
	template<>
	void ReSerialize(const Serialzer_t& Archive, FPluginDescription& desc);
}