#pragma once
#include"Core/ArcCore.h"
#include"SequentialCollection.h"
#include<mutex>
#include<condition_variable>
#include<optional>

namespace Concurrency {
/*
*	a thread safe sequential container wrapper class
*	uses macro grained mutexes
* container must use the same data type
* @EntryInspector : a member function of Container that is used to access the proper entry in it, top, back, front ...
* container class must at least provide the same interface as a sequential container class
*Container class: pop, size, empty, top, push, ::value_type implementation is expected
*/
template<typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
class NODISCARD ARC_API LB_SequentialContainer : public ISequentialCollection<Data_t>
{
	static_assert(std::is_same_v<Data_t, Container::value_type>, "container adaptors require consistent types");

	//thread ignorant swap, call global function for thread safety
	void swap(LB_SequentialContainer& other)noexcept;
protected:
	mutable std::mutex mtx;
	std::condition_variable ConditionVariable;
	Container InternalSQ;//internal sequential container

public:
	
	using container_type = Container;

	LB_SequentialContainer() = default;
	LB_SequentialContainer(const LB_SequentialContainer& other);
	LB_SequentialContainer(LB_SequentialContainer&& other) noexcept;
	LB_SequentialContainer& operator=(const LB_SequentialContainer& other);
	LB_SequentialContainer& operator=(LB_SequentialContainer&& other) noexcept;

	//* SequntialContainer API begin

	 size_t size() const;
	 bool empty()const;

	 void push(const Data_t& data);
	 void push(Data_t&& data)noexcept;

	 NODISCARD Data_t pop();
	 //* SequntialContainer API ends

	//checks if we have items then pops, return true on success
	 NODISCARD std::optional<Data_t> try_pop()noexcept;
};

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
inline void LB_SequentialContainer<Data_t , Container, EntryInspector>::swap(LB_SequentialContainer& other) noexcept
{
	InternalSQ.swap(other.InternalSQ);
}

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
inline LB_SequentialContainer<Data_t , Container, EntryInspector>::LB_SequentialContainer(const LB_SequentialContainer& other) : mtx(), ConditionVariable()
{
	std::lock_guard lock(other.mtx);
	InternalSQ = other.InternalSQ;
}

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
inline LB_SequentialContainer<Data_t , Container, EntryInspector>::LB_SequentialContainer(LB_SequentialContainer&& other) noexcept:	mtx() ,	ConditionVariable()
{
	std::lock_guard lock(other.mtx);
	InternalSQ = std::move(other.InternalSQ);
}

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
inline LB_SequentialContainer<Data_t , Container, EntryInspector>& LB_SequentialContainer<Data_t , Container, EntryInspector>::operator=(const LB_SequentialContainer& other)
{
	if (this != &other)
	{
		LB_SequentialContainer<Data_t , Container, EntryInspector> temp{ other };
		swap(temp);
	}
	return *this;
}

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
inline LB_SequentialContainer<Data_t , Container, EntryInspector>& LB_SequentialContainer<Data_t , Container, EntryInspector>::operator=(LB_SequentialContainer&& other) noexcept
{
	if (this != &other)
	{
		LB_SequentialContainer<Data_t , Container, EntryInspector> temp{ std::move(other) };
		swap(temp);
	}
	return *this;
}

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
inline size_t LB_SequentialContainer<Data_t , Container, EntryInspector>::size() const
{
	std::lock_guard lock(mtx);
	return InternalSQ.size();
}

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
inline bool LB_SequentialContainer<Data_t , Container, EntryInspector>::empty() const
{
	std::lock_guard lock(mtx);
	return InternalSQ.empty();
}

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
inline void LB_SequentialContainer<Data_t , Container, EntryInspector>::push(const Data_t& data)
{
	std::lock_guard lock(mtx);
	InternalSQ.push(data);
	ConditionVariable.notify_one();
}

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
inline void LB_SequentialContainer<Data_t , Container, EntryInspector>::push(Data_t&& data) noexcept
{
	std::lock_guard lock(mtx);
	InternalSQ.push(std::move(data));
	ConditionVariable.notify_one();
}

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
inline Data_t LB_SequentialContainer<Data_t , Container, EntryInspector>::pop()
{
	//unique lock allows mulltiple locking and unlocking >> increases concurrent potential
	std::unique_lock lock(mtx);
	ConditionVariable.wait(lock, [&] {return !InternalSQ.empty(); });
	Data_t popped{ (InternalSQ.*EntryInspector)() };
	InternalSQ.pop();
	return popped;
}

template<typename Data_t, class Container, Data_t& (Container::* EntryInspector)()>
inline std::optional<Data_t> LB_SequentialContainer<Data_t, Container, EntryInspector>::try_pop() noexcept
{
	std::lock_guard lock(mtx);
	if (!InternalSQ.empty())
	{
		std::optional<Data_t> res{ (InternalSQ.*EntryInspector)() };
		InternalSQ.pop();
		return res;
	}
	return std::nullopt;
}

template <typename Data_t, class Container, Data_t&(Container::* EntryInspector)()>
void swap(LB_SequentialContainer<Data_t , Container, EntryInspector>& first, LB_SequentialContainer<Data_t, Container , EntryInspector>& second)
{
	//lock mutexes 
	std::lock(first.mtx, second.mtx);
	//adopt mutexes , avoid relocking and ensure unlocking on exit
	std::lock_guard L1(first.mtx, std::adopt_lock);
	std::lock_guard L2(second.mtx, std::adopt_lock);

	//swap
	first.swap(second);
}
}