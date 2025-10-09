#pragma once
#include"Core/ArcCore.h"
#include<atomic>
#include<memory>

//TODO: complete

namespace Concurrency {
	/*0
	*	a queue class, that uses atomics
	*	must contain atleast an atomic variable
	*  uses atomic memory ordering instead of mutexes
	* this class uses reference counting to manage deleting objects
	*/
	template<typename Data_t>
	class NODISCARD ARC_API LFQueue
	{
	protected:
		struct Node
		{
			std::atomic<std::shared_ptr<Data_t>> Data{};
			std::shared_ptr<Node> NextNode{};
			Node() = default;
			explicit Node(Data_t val) : Data{ std::make_shared<Data_t>(std::move(val)) } {};
			Node(Node&&)noexcept = default;
			Node(const Node&) = default;
		};

		std::atomic<std::shared_ptr<Node>> Head;
		std::atomic<Node*> Tail;

	public:
		LFQueue() :Head{ std::make_shared<Node>() }, Tail{Head.load(std::memory_order_relaxed).get()}
		{
			//start with dummy pointers
		}
		void push(const Data_t& data)
		{
			//create new node
			std::shared_ptr<Node> New_Node{ std::make_shared<Node>(Node{}) };//create a new dummy node
			auto DataPtr{ std::make_shared<Data_t>(data) };
			
			while (true)
			{
				Node* Old_Tail{ Tail.load(std::memory_order_acquire) };
				std::shared_ptr<Data_t> Expected{};
				if (Old_Tail->Data.compare_exchange_strong(Expected, DataPtr,std::memory_order_relaxed))
				{
					Old_Tail->NextNode = New_Node;
					break;
				}
				//TODO: we can add here a helper code to avoid extra waiting
			}
			Tail.store(New_Node.get(),std::memory_order_release);
		}
		void push(Data_t&& data)noexcept
		{
			//create new node
			Node* New_Node{ new Node{} };//create a new dummy node
			auto DataPtr{ std::make_shared<Data_t>(std::move(data)) };

			while (true)
			{
				Node* Old_Tail{ Tail.load(std::memory_order_acquire) };
				std::shared_ptr<Data_t> Expected{};
				if (Old_Tail->Data.compare_exchange_strong(Expected, DataPtr, std::memory_order_relaxed))
				{
					Old_Tail->NextNode.reset(New_Node);//adopt the pointer
					break;
				}
			}
			Tail.store(New_Node, std::memory_order_release);
		 }

		 //pop the stack, if the stack is empty then we will return a nullptr
		NODISCARD std::shared_ptr<Data_t> pop()
		{
			std::shared_ptr<Node> Old_Head{ Head.load(std::memory_order_acquire) };
			
			do
			{
				if (Old_Head.get() == Tail.load(std::memory_order_acquire))//queue is empty, ensure that tail in in the release chain
				{
					return nullptr;
				}
			} while (!Head.compare_exchange_strong(Old_Head, Old_Head->NextNode, std::memory_order_release, std::memory_order_acquire));
			return std::move(Old_Head->Data);
		}
	};

	

}