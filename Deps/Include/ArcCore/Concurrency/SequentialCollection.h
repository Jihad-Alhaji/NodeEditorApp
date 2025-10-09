#pragma once
#include"Core/ArcCore.h"
/*
*interface class for all objects that represents a Sequential Collection of objects
*/
template<typename Data_t>
class ARC_API ISequentialCollection
{
public:
	using value_type = Data_t;

	virtual size_t size() const = 0;
	virtual bool empty()const = 0;

	virtual void push(const Data_t& data) = 0;
	virtual void push(Data_t&& data)noexcept = 0;

	//the univerdsal pop, exposed to the called, can be wait or wait free function
	virtual Data_t pop() = 0;
};