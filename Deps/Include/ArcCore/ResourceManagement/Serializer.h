#pragma once
#include"Core/ArcCore.h"
#include<memory>
#include"nlohmann.hpp"

namespace ResourceManagement
{
	namespace Serialization
	{

		using Serialzer_t = nlohmann::json;

		//def serialization functions
		template<class serializer_t, class Obj_t>
		inline void Serialize(serializer_t& serializer, const Obj_t& obj)
		{
			serializer = obj;
		}

		template<class reserializer_t, class Obj_t>
		inline void ReSerialize(const reserializer_t& reserializer, Obj_t& obj)
		{
			obj = reserializer;
		}

		//custom serialization for smart ptr, specialize for required types
		template<class serializer_t, class Obj_t>
		void Serialize(serializer_t& serializer, std::shared_ptr<Obj_t> obj);

		template<class reserializer_t, class Obj_t>
		void ReSerialize(const reserializer_t& reserializer, std::shared_ptr<Obj_t>& obj);

		//save an archive to a file directly
		bool SaveToFile(const Serialzer_t& Archive, std::string_view path, bool AsBinary = false);

		//loads an archive from a file directly
		bool LoadFromFile(Serialzer_t& Archive, std::string_view path, bool AsBinary = false);
	}
}

//saves a property to the archive with the same name
#define SaveProp(archive, prop) ResourceManagement::Serialization::Serialize(archive[#prop],prop)

//safly loads a simple property
#define LoadProp(archive, prop) { auto prop_Iter = archive.find(#prop); if (prop_Iter != archive.end()){ ResourceManagement::Serialization::ReSerialize(prop_Iter.value(),prop); }}

//saves a property of an object
#define SaveChildProp(archive,owner,prop) ResourceManagement::Serialization::Serialize(archive[#prop],owner.prop)

//loads a property of an object
#define LoadChildProp(archive,owner,prop) { auto prop_Iter = archive.find(#prop); if (prop_Iter != archive.end()){ ResourceManagement::Serialization::ReSerialize(prop_Iter.value(),owner.prop); }}

//safely loads an array
#define LoadProp_Array(archive, prop) \
{ \
	auto prop_Iter = archive.find(#prop);\
	if (prop_Iter != archive.end())\
	{  \
		prop.resize(prop_Iter.value().size());\
		for (size_t i = 0; i < prop_Iter.value().size(); i++)\
		{\
			ResourceManagement::Serialization::ReSerialize(prop_Iter.value()[i], prop[i]);\
		}\
	}\
}

//saves an array
#define SaveProp_Array(archive, prop) \
{ \
	for (size_t i = 0; i < prop.size(); i++)\
	{\
		ResourceManagement::Serialization::Serialize(archive[#prop][i], prop[i]);\
	}\
}

#define LoadChildProp_Array(archive,owner, prop) \
{ \
	auto prop_Iter = archive.find(#prop);\
	if (prop_Iter != archive.end())\
	{  \
		owner.prop.resize(prop_Iter.value().size());\
		for (size_t i = 0; i < prop_Iter.value().size(); i++)\
		{\
			ResourceManagement::Serialization::ReSerialize(prop_Iter.value()[i], owner.prop[i]);\
		}\
	}\
}

#define SaveChildProp_Array(archive,owner, prop) \
{ \
	for (size_t i = 0; i < owner.prop.size(); i++)\
	{\
		ResourceManagement::Serialization::Serialize(archive[#prop][i], owner.prop[i]);\
	}\
}