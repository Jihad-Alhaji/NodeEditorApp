#pragma once
#include"ResourceManagement/Serializer.h"
#include"imgui.h"
namespace ResourceManagement
{
	namespace Serialization
	{
		//serialization helpers for imgui ve types
		template<class serializer_t>
		inline void Serialize(serializer_t& serializer, const ImVec2& vec)
		{
			serializer[0] = vec.x;
			serializer[1] = vec.y;
		}

		template<class reserializer_t>
		inline void ReSerialize(const reserializer_t& reserializer, ImVec2& vec)
		{
			vec.x = reserializer[0];
			vec.y = reserializer[1];
		}
		template<class serializer_t>
		inline void Serialize(serializer_t& serializer, const ImVec4& vec)
		{
			serializer[0] = vec.x;
			serializer[1] = vec.y;
			serializer[2] = vec.z;
			serializer[3] = vec.w;
		}

		template<class reserializer_t>
		inline void ReSerialize(const reserializer_t& reserializer, ImVec4& vec)
		{
			vec.x = reserializer[0];
			vec.y = reserializer[1];
			vec.z = reserializer[2];
			vec.w = reserializer[3];
		}
	}
}