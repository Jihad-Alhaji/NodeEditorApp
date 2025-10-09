#pragma once
#include"Core/ArcCore.h"
#include<functional>
#include<vector>
#include"Hash.h"

//helpers to get functions ids for users

#define GetFunctionID(funcPtr) GetPointerHash(funcPtr)
#define GetMemFunctionID(objectPtr, MemFunc) GetMemeberFunctionHash(objectPtr, MemFunc)
#define GetCustomNameID(name) std::hash<const char*>()(#name)
#define BindMemFunction(objectPtr, ScopedMemFunc) std::bind_front(&ScopedMemFunc,objectPtr)
namespace Structures
{

	//thrown when an unbound delegate is called
	class bad_delegate_call : public std::bad_function_call
	{
	public:
		NODISCARD const char*  what() const noexcept override {
			// return pointer to message string
			return "bad Delegate call: cannot call unbound delegate!";
		}
	};
	
	/*
	* delegates its functionalty 
	* interface
	*/
	template<typename Return_t, typename ...Args>
	class IDynamicDelegate
	{
	public:
		using  Delegate_t = typename std::function<Return_t(Args...)>;
		virtual void Bind(Delegate_t&& d) = 0;
		virtual void UnBind() = 0;
		virtual Return_t Call(Args&&... args) = 0;
		virtual bool IsBound()const = 0;

		Return_t operator()(Args&&... args) {
			return Call(std::forward<Args>(args)...);
		};
	};
	/*
	* classic mono delegate
	*/
	template<typename Return_t, typename ...Args>
	class TDynamicDelegate: public IDynamicDelegate<Return_t,Args...>
	{
		IDynamicDelegate<Return_t, Args...>::Delegate_t Delegate;
	public:
		void Bind(IDynamicDelegate<Return_t, Args...>::Delegate_t&& d)override
		{
			Delegate = std::move(d);
		}
		void UnBind()override
		{
			Delegate = nullptr;
		}
		Return_t Call(Args&&... args)override
		{
			try
			{
				return Delegate(std::forward<Args>(args)...);
			}
			catch (const std::bad_function_call&)
			{
				throw bad_delegate_call();
			}
		}
		bool IsBound()const override
		{
			return Delegate.operator bool();
		}
	};

	/*
	* allow handelling of multi delegates at the same time
	*/
	template<typename ...Args>
	class TMultiDelegate : public IDynamicDelegate<void, Args...>
	{
		using DelegatePak_t = typename std::pair<size_t, std::function<void(Args...)>>;

		std::vector<DelegatePak_t> Delegates;
		
		//hidden to prevent adding delegates without ids
		void Bind(std::function<void(Args...)>&& d)override {}
	public:
		
		//for the id use make pointer hash / make memfunction hash to make unique ids
		void Bind(size_t ID , std::function<void(Args...)>&& d)
		{
			Delegates.push_back({ ID,std::move(d) });
		}

		void UnBind(size_t id)
		{
			auto iter = std::find_if(Delegates.begin(), Delegates.end(), [id](const DelegatePak_t& val)->bool
			{
				return val.first == id;
			});
			if (iter != Delegates.end())
			{
				Delegates.erase(iter);
			}
		}
		
		void UnBind()override
		{
			Delegates.clear();
		}
		
		void Call(Args&&... args)override
		{
			try
			{
				for (auto& d : Delegates)
				{
					d.second(std::forward<Args>(args)...);
				}
			}
			catch (const std::bad_function_call&)
			{
				throw bad_delegate_call();
			}
		}
		//when atleast we have a one valid delegate
		bool IsBound()const override
		{
			return !Delegates.empty();
		}
	};

}
