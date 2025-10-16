#pragma once

namespace ArcCore
{
	#define GetPointerHash(ptr) std::_Hash_representation<void*>(ptr)
	#define GetMemeberFunctionHash(objectPtr, MemFunc) ArcCore::CombineHash(GetPointerHash(objectPtr), std::hash<const char*>()(#MemFunc))

	size_t CombineHash(size_t h1, size_t h2);
}

template <typename... Ts>
inline void hash_combine(std::size_t& seed, const Ts&... Args) {
	((seed ^= std::hash<Ts>{}(Args)+0x9e3779b9 + (seed << 6) + (seed >> 2)), ...);
}