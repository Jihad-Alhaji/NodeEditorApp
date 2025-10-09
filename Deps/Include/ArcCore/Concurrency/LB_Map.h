#pragma once
#include"LB_List.h"
#include<vector>
#include<memory>
namespace Concurrency {
template<typename key_t, typename value_t, typename hash_t = std::hash<key_t>>
class NODISCARD ARC_API LB_MAP
{
	
	using Bucket = LB_List<key_t, value_t>;

	std::vector<std::unique_ptr<Bucket>> Buckets;
	hash_t Hasher;

	Bucket& GetBucket(const key_t& key)const
	{
		//an optimization to increase potencial of concurrency
		return *Buckets[Hasher(key) % Buckets.size()];
	}

public:
	LB_MAP(size_t num = 19, hash_t&& hasher = hash_t()) : Buckets{ num }, Hasher{ std::move(hasher) }
	{
		for (auto& elem : Buckets)
		{
			elem.reset(new Bucket());
		}
	}

	LB_MAP(const LB_MAP&) = delete;
	LB_MAP& operator=(const LB_MAP&) = delete;

	NODISCARD std::optional<value_t> find(const key_t& key)const
	{
		return GetBucket(key).find(key);
	}
	//add or updates a pair
	void set(const key_t& key, const value_t& value)
	{
		GetBucket(key).set(key, value);
	}
	void erase(const key_t& key)
	{
		GetBucket(key).erase(key);
	}
};
}