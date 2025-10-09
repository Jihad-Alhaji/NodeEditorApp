#pragma once

//interface for plugin system
class IPluginInterface
{
public:
	virtual ~IPluginInterface() = default;

	// Called when the plugin is loaded
	virtual void OnLoad(void* context) {};

	// Called when the plugin is unloaded
	virtual void OnUnload() {};

	// Returns the name of the plugin
	virtual const char* GetName() const = 0;
};

using GetPluginFunc_t = IPluginInterface * (*)();

#define GET_PLUGIN_FUNC_NAME "GetPlugin"
#define DECLARE_PLUGIN(API) extern "C" API IPluginInterface* GetPlugin();
#define DEFINE_PLUGIN(CLASS_NAME) IPluginInterface* GetPlugin() {\
	static CLASS_NAME* instance{new CLASS_NAME()}; \
	return instance; \
} 