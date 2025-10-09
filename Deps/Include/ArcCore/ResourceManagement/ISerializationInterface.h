#pragma once
#include"ResourceManagement/Serializer.h"
namespace ResourceManagement
{
	namespace Serialization
	{
		class ISerializationInterface
		{
		public:
			using Serialzer_t = typename Serialization::Serialzer_t;
			virtual bool Load(const Serialzer_t& Archive) { return true; };
			virtual void Save(Serialzer_t& Archive) {};
			virtual void Unload() {};
		};

		//save an object to a file directly
		bool SaveToFile(ISerializationInterface& obj, std::string_view path, bool AsBinary = false);

		//loads an object from a file directly
		bool LoadFromFile(ISerializationInterface& obj, std::string_view path, bool AsBinary = false);

		//custom save and load for iserializatoin interface objects
		template<class serializer_t>
		inline void Serialize(serializer_t& serializer, const ISerializationInterface& obj)
		{
			obj.Save(serializer);
		}

		template<class reserializer_t>
		inline void ReSerialize(const reserializer_t& reserializer, ISerializationInterface& obj)
		{
			obj.Load(reserializer);
		}
	}
}
