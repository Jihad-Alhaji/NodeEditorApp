#pragma once

namespace ArcCore
{
	#define GetPointerHash(ptr) std::_Hash_representation<void*>(ptr)
	#define GetMemeberFunctionHash(objectPtr, MemFunc) ArcCore::CombineHash(GetPointerHash(objectPtr), std::hash<const char*>()(#MemFunc))

	size_t CombineHash(size_t h1, size_t h2);
}