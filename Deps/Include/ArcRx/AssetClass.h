#pragma once
#include"Macros.h"
#include<string>
#include<memory>
#include<functional>
#include<map>

namespace ResourceManagement
{
	class IAsset;
	struct ARCRX_API IAssetClass
	{
		using AsssetName_t = std::string;
		using AssetInstancer_t = std::function<std::shared_ptr<IAsset>(void)>;

		AsssetName_t ClassName;
		AssetInstancer_t AssetInstancer;

		//defauly asset classes
		static std::map<AsssetName_t, std::shared_ptr<IAssetClass>> AssetClasses;

		static std::shared_ptr<IAsset> MakeInstance(std::shared_ptr<IAssetClass> fromClass);
		static std::shared_ptr<IAsset> MakeInstance(AsssetName_t fromClassName);
		static void SetAssetClass(std::shared_ptr<IAsset> asset, std::shared_ptr<IAssetClass> ClassType);
	};
	template<class T>
	std::shared_ptr<T> TMakeAssetInstance(IAssetClass::AsssetName_t fromClassName)
	{
		static_assert(std::is_base_of_v<IAsset, T>, "Type must be a child of IAsset class!");
		return std::dynamic_pointer_cast<T>(IAssetClass::MakeInstance(fromClassName));
	}
}

#ifndef DEFINE_ASSET
//defines required information for an asset, put it in a .cpp file
#define DEFINE_ASSET(Asset_t)\
	std::shared_ptr<Asset_t::Asset_t##GeneratedClass> Asset_t::Asset_t##GeneratedClass::StaticClass = InitClass();\
	std::shared_ptr<Asset_t::Asset_t##GeneratedClass> Asset_t::Asset_t##GeneratedClass::InitClass()\
		{\
			auto res = std::make_shared<Asset_t##GeneratedClass>();\
			AssetClasses[#Asset_t] = res;\
			return res;\
		}\
	Asset_t::Asset_t##GeneratedClass::Asset_t##GeneratedClass()\
	{\
			ClassName =  #Asset_t;\
			AssetInstancer = []()->std::shared_ptr<ResourceManagement::IAsset> \
				{\
					auto insAsset = std::make_shared<Asset_t>();\
					SetAssetClass(insAsset,StaticClass);\
					return insAsset;\
				};\
	}
#endif
#ifndef INIT_ASSET
	#define INIT_ASSET(Asset_t) Asset_t::Asset_t##GeneratedClass::StaticClass->StaticClass;
#endif
#ifndef DECLARE_ASSET
//decalres required information for an asset, put it in the beginning of your asset, must be public
#define DECLARE_ASSET(Asset_t)\
	struct  Asset_t##GeneratedClass : public ResourceManagement::IAssetClass\
	{\
		public:\
				static std::shared_ptr<Asset_t##GeneratedClass> InitClass();\
		public:\
		static std::shared_ptr<Asset_t##GeneratedClass> StaticClass;\
		Asset_t##GeneratedClass();\
	};
#endif

