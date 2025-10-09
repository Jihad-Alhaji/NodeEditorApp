#pragma once
#include"Core/ArcCore.h"
#include"SequentialCollection.h"
#include<atomic>
#include<memory>



namespace Concurrency {
	/*0
	*	a stack class, that uses atomics
	*	must contain atleast an atomic variable
	*  uses atomic memory ordering instead of mutexes
	* this class uses reference counting to manage deleting objects
	*/
	template<typename Data_t>
	class NODISCARD ARC_API LFStack 
	{
	protected:
		struct Node
		{
			std::shared_ptr<Data_t> Data{};
			std::shared_ptr<Node> NextNode{};
			explicit Node(Data_t val) : Data{ std::make_shared<Data_t>(std::move(val)) } {};
			Node(Node&&)noexcept = default;
			Node(const Node&) = default;
		};

		std::atomic<std::shared_ptr<Node>> Head;
	public:
		 void push(const Data_t& data) ;
		 void push(Data_t&& data)noexcept ;

		 //pop the stack, if the stack is empty then we will wait for entries
		  NODISCARD std::shared_ptr<Data_t> pop() ;
	};

	template<typename Data_t>
	void LFStack<Data_t>::push(const Data_t& data)
	{
		const auto NewNode{ std::make_shared<Node>(Node{data}) };
		NewNode->NextNode = Head.load(std::memory_order_relaxed);
		while (!Head.compare_exchange_weak(NewNode->NextNode, NewNode, std::memory_order_release,std::memory_order_relaxed));
	}

	template<typename Data_t>
	inline void LFStack<Data_t>::push(Data_t&& data) noexcept
	{
		const auto NewNode{ std::make_shared<Node>(Node{std::move(data)}) };
		NewNode->NextNode = Head.load(std::memory_order_relaxed);
		while (!Head.compare_exchange_weak(NewNode->NextNode, NewNode, std::memory_order_release, std::memory_order_relaxed));
	}

	template<typename Data_t>
	inline std::shared_ptr<Data_t> LFStack<Data_t>::pop()
	{
		auto Old_Head{ Head.load(std::memory_order_relaxed) };
		//multi thread access to old head ptr is safe because of the internal ref counting, no deallocation will occur till are are done with
		while (Old_Head && !Head.compare_exchange_weak(Old_Head, Old_Head->NextNode,std::memory_order_acquire,std::memory_order_relaxed));
		return Old_Head? std::move(Old_Head->Data): nullptr;
	}


	//lock free stack with split reference management
	template<class T>
	class LFStack_SplitRef
	{
		struct node;
		struct CountedNode_Ptr
		{
			//used by what ever thing that reference the pointer
			int ExternalCount;
			node* ptr;
		};

		struct node
		{
			std::shared_ptr<T> Data;
			CountedNode_Ptr next;
			std::atomic<int> InternalCount;
		};
		//lock free on x32 machines
		std::atomic<CountedNode_Ptr> Head;
	public:
		~LFStack_SplitRef()
		{
			//empty the stack
			while (pop());
		}

		void push(const T& value)
		{
			CountedNode_Ptr newNode{};
			newNode.ptr = new node{};
			newNode.ptr->Data = std::make_shared<T>(value);
			newNode.ptr->next = Head.load(std::memory_order_relaxed);
			newNode.ExternalCount = 1;
			while (!Head.compare_exchange_weak(newNode.ptr->next, newNode, std::memory_order_release, std::memory_order_relaxed));
		}
		void push(T&& value)
		{
			CountedNode_Ptr newNode{};
			newNode.ptr = new node{};
			newNode.ptr->Data = std::make_shared<T>(std::move(value));
			newNode.ptr->next = Head.load(std::memory_order_relaxed);
			newNode.ExternalCount = 1;
			while (!Head.compare_exchange_weak(newNode.ptr->next, newNode, std::memory_order_release, std::memory_order_relaxed));
		}
		std::shared_ptr<T> pop()
		{
			auto oldHead{ Head.load(std::memory_order_relaxed) };
			while (true)
			{
				increase_head_count(oldHead);
				node* ptr{ oldHead.ptr };
				if (!ptr)
				{
					return nullptr;
				}
				if (Head.compare_exchange_strong(oldHead, ptr->next, std::memory_order_relaxed))
				{
					auto res{ std::move(ptr->Data) };
					const auto CountDelta{ oldHead.ExternalCount - 2 };
					//to ensure ptr->Data move happens before the delete
					if (ptr->InternalCount.fetch_add(CountDelta, std::memory_order_release) == -CountDelta)
					{
						delete ptr;
					}
					return res;
				}
				else if (ptr->InternalCount.fetch_add(-1, std::memory_order_relaxed) == 1)
				{
					ptr->InternalCount.load(std::memory_order_acquire);//to ensure ptr->Data move happens before the delete
					delete ptr;
				}
			}
		}
	private:

		void increase_head_count(CountedNode_Ptr& oldCounter)
		{
			CountedNode_Ptr newCounter{};
			do
			{
				newCounter = oldCounter;
				++newCounter.ExternalCount;
			} while (!Head.compare_exchange_strong(oldCounter, newCounter, std::memory_order_acquire, std::memory_order_relaxed));
			oldCounter.ExternalCount = newCounter.ExternalCount;
		}
	};

}