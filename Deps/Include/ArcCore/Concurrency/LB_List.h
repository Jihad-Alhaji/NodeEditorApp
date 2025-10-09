#pragma once
#include"Core/ArcCore.h"
#include<shared_mutex>
#include<list>
#include<optional>

namespace Concurrency {
template<typename key_t, typename value_t>
class NODISCARD ARC_API LB_List
{
	using pair_t = std::pair<key_t, value_t>;
	using pairList_t = std::list<pair_t>;
	using iterator_t = typename pairList_t::iterator ;
	using const_iterator_t = typename pairList_t::const_iterator;

	pairList_t Data;
	mutable std::shared_mutex mtx;
	//non blocking operations, should be under the protection of a lock
	iterator_t find_iterator(const key_t& key)
	{
		return std::find_if<iterator_t>(Data.begin(), Data.end(), [&](const pair_t& item) {return item.first == key; });
	}
	const_iterator_t find_iterator(const key_t& key)const
	{
		return std::find_if<const_iterator_t>(Data.begin(), Data.end(), [&](const pair_t& item) {return item.first == key; });
	}

public:
	//retuns a optional copy the item 
	NODISCARD std::optional<value_t> find(const key_t& key)const
	{
		std::shared_lock<std::shared_mutex> lock(mtx);
		if (auto iter(find_iterator(key)); iter != Data.end())
		{
			return iter->second;
		}
		else
		{
			return std::nullopt;
		}
	}
	//add or updates a pair
	void set(const key_t& key, const value_t& value)
	{
		std::unique_lock<std::shared_mutex> lock{ mtx };

		if (auto iter{ find_iterator(key) }; iter == Data.end())
		{
			Data.push_back(pair_t{ key, value });
		}
		else
		{
			iter->second = value;
		}
	}
	void set(const key_t& key, value_t&& value)
	{
		std::unique_lock<std::shared_mutex> lock{ mtx };

		if (auto iter{ find_iterator(key) }; iter == Data.end())
		{
			Data.push_back(pair_t{ key, std::move(value) });
		}
		else
		{
			iter->second = std::move(value);
		}
	}
	//add or updates a pair, use this version if you want to move the key and/or value to the list
	void set(pair_t&& pair)
	{
		std::unique_lock<std::shared_mutex> lock{ mtx };

		if (auto iter{ find_iterator(pair->first) }; iter == Data.end())
		{
			Data.push_back(std::move(pair));
		}
		else
		{
			iter->second = std::move(pair->second);
		}
	}
	void erase(const key_t& key)
	{
		std::unique_lock<std::shared_mutex> lock{ mtx };

		if (auto iter{ find_iterator(key) }; iter != Data.end())
		{
			Data.erase(iter);
		}
	}
};
}
