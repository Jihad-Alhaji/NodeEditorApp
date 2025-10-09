#pragma once
#include"Macros.h"
#include<string>
#include<memory>
#include<nlohmann.hpp>
#include"ResourceManagement/ISerializationInterface.h"
#include"AssetClass.h"
#include"LogSystem.h"
#include"AssetManager.h"
namespace ResourceManagement
{
	/*
	* an interface to what should be managed as a resource
	* all game  objects should inherit from this
	* can be serialized/reserialized
	* centally managed
	* load/unload/load on demand
	* runtime type features
	*/
	class  ARCRX_API IAsset : public Serialization::ISerializationInterface, public std::enable_shared_from_this<IAsset>
	{
	public:
		DECLARE_ASSET(IAsset);
		virtual ~IAsset() {};
	private:
		static size_t AssetsCounter;//used to create IDs
	protected:

		std::string DisplayName{ "unnamed_asset_" + std::to_string(++AssetsCounter) };
		std::string AssetPath  {};
		std::shared_ptr<IAssetClass> AssetClass;

		bool bPendingDestroy : 1{0};//whether we should be destryed
		bool bLoaded : 1{0};//are we loaded and usable
		bool bLoadOnDemand : 1{1};//should we be loaded on demand or be loaded initially
		bool bRegistered : 1{0};//are we regitered in the manager map
		bool bShouldEverSave : 1{1};//should we ever save on unload
		bool bNeedToSave : 1{0};//have we changed > need to save


		using Serializer_t = nlohmann::json;

		virtual bool Load(const Serializer_t& Archieve)override;

		virtual void Save(Serializer_t& Archieve)override;
	public:
		static inline const char* const AssetFileExtension{ ".AA" };
		virtual void Unload()override;

		//open a file and load the asset from it
		bool LoadFromFile();

		//allow for loading directly from an processed archive
		bool LoadFromFile(const Serializer_t& Archieve);

		//saves to a file
		void SaveToFile();

		//saves to an archive
		void SaveToFile(Serializer_t& Archieve);

		bool IsLoaded()const { return bLoaded; }
		bool IsPendingDestroy()const { return bPendingDestroy; }
		bool IsLoadOnDemand()const { return bLoadOnDemand; }
		bool IsRegistered()const { return bRegistered; }
		bool ShouldSave()const { return bNeedToSave; }
		bool ShouldEverSave()const { return bShouldEverSave; }

		std::string_view GetDispayName()const { return DisplayName; }
		std::string_view GetPath()const { return AssetPath; }
		std::shared_ptr<IAssetClass> GetClass()const { return AssetClass; }

		bool SetDisplayName(std::string_view newName);
		void SetPath(std::string_view path);
		void SetLoadOnDemand(bool val);
		void SetShouldEverSave(bool val);
		void SetAsNeedToSave() { bNeedToSave = true; }

		//makes a new shared pointer from this instance, shared this if you will
		template<class T>
		std::shared_ptr<T> MakeSharedPtr()
		{
			return std::dynamic_pointer_cast<T>(shared_from_this());
		}
		static size_t GetAssetsCount()noexcept { return AssetsCounter; }
		friend class AssetManager;

		static inline std::map<std::string, IAsset*> DefaultAssets;

		friend struct IAssetClass;
		friend class AssetManager;
	};

	//these are partial spesialization and need to be in the header file
	namespace Serialization
	{
		//asset pointer serialization
		template<class Serializer>
		void Serialize(Serializer& archive, const std::shared_ptr<IAsset>& Assetptr)
		{

			if (Assetptr)
			{
				//used to get the asset if already loaded
				archive["AssetName"] = Assetptr->GetDispayName();

				//used to load the asset if not loaded, this requires us to figure the type dynamically
				archive["AssetPath"] = Assetptr->GetPath();
				archive["AssetClass"] = Assetptr->GetClass()->ClassName;
			}
			else
			{
				archive["AssetName"] = "";
				archive["AssetPath"] = "";
				archive["AssetClass"] = "";
			}

		}

		template<class Serializer>
		void ReSerialize(const Serializer& js, std::shared_ptr<IAsset>& Assetptr)
		{
			const std::string assetName = js["AssetName"];//display name
			const std::string assetPath = js["AssetPath"];
			const std::string AssetClassName = js["AssetClass"];
			std::shared_ptr<IAsset> asset{};
			if (!assetName.empty())
			{
				asset = AssetManager::GetAsset(assetName);
				if (asset)
				{
					Assetptr = std::move(asset);
					return;
				}
			}
			if (!assetPath.empty() && !AssetClassName.empty())
			{
				//try loading the asset from disk
				//NOTE: we need to create correct type of asset here and need to check for inheritance errors
				ArcNote("AssetPointer reserialization : will load referenced asset from disk : " + assetPath + IAsset::AssetFileExtension);
				asset = AssetManager::LoadAssetFromFile(assetPath);
				if (asset)
				{
					Assetptr = std::move(asset);
				}
				else
				{
					ArcError("AssetPointer reserialization error: asset could not be loaded with proper type name: " + assetName);
				}
				return;
			}
			Assetptr = nullptr;
		}
		//saves asset data inline
		template<class Serializer>
		void SerializeSubObject(Serializer& archive, std::shared_ptr<IAsset> SubObj)
		{
			if (SubObj)
			{
				SubObj->SetAsNeedToSave();
				SubObj->SaveToFile(archive);
			}
		}

		template<class Serializer>
		std::shared_ptr<IAsset> ReSerializeSubObject(const Serializer& archive)
		{
			std::shared_ptr<IAsset> res = IAssetClass::MakeInstance(archive["AssetClass"]);
			if (res && res->LoadFromFile(archive))
			{
				return res;
			}
			return nullptr;
		}

		//asset pointer serialization
		template<class Serializer>
		void SerializeAssetPtr(Serializer& archive, const std::shared_ptr<IAsset>& Assetptr)
		{
			Serialize(archive, Assetptr);
		}

		//asset pointer serialization
		template<class AssetType_t, class Serializer>
		std::shared_ptr<AssetType_t> ReSerializeAssetPtr(Serializer& archive)
		{
			std::shared_ptr<IAsset> res{};
			ReSerialize(archive, res);
			return std::dynamic_pointer_cast<AssetType_t>(res);
		}
	}
}

