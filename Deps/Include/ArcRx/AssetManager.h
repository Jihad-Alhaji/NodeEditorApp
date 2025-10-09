#pragma once
#include"Macros.h"
#include"Concurrency/LB_Map.h"
#include"IAsset.h"
#include<filesystem>
namespace ResourceManagement
{

	//workflow:
	// 1. create the asset of the type you want, set the path for the asset
	// 2. register the asset
	// 3 . load when wanted
	class  ARCRX_API AssetManager
	{
		static Concurrency::LB_MAP<std::string, std::shared_ptr<IAsset>> Assets_Map;

	public:
		using path_t = typename std::filesystem::path;
		//discover assets
		//
		static bool RegisterAsset(const std::shared_ptr<IAsset>& asset);

		static void UnRegisterAsset(std::shared_ptr<IAsset> asset);
		static void UnRegisterAsset(std::string_view assetName);

		static std::shared_ptr<IAsset> GetAsset(std::string_view assetname);

		static bool UpdateAssetKey(std::shared_ptr<IAsset> asset, std::string_view newName);
		static bool UpdateAssetKey(std::string_view assetName, std::string_view newName);

		static bool UnloadAsset(std::shared_ptr<IAsset> asset);
		static bool UnloadAsset(std::string_view assetName);

		//loads a registered asset
		static bool LoadAsset(std::shared_ptr<IAsset> asset);
		static bool LoadAsset(std::string_view assetName); 

		//load an asset directly from a path and register it
		static std::shared_ptr<IAsset> LoadAssetFromFile(std::string assetPath);

		//loads all assets found in the directory and registers them
		static size_t LoadAllAssetsInDir(path_t Dir, bool recrusive = true);

	};
}