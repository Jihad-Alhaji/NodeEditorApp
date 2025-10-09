#pragma once
#include"SequentialCollection.h"
#include<mutex>
#include<condition_variable>
#include"SingleLinkedNode.h"
#include<atomic>
#include<optional>
namespace Concurrency
{
	template<typename Data_t>
	class NODISCARD ARC_API LB_FineGrainedQueue : public ISequentialCollection<Data_t>
	{
		static_assert(std::is_default_constructible_v<Data_t>, "LB_FineGrainedQueue Expectes a default constructble data trpe!");
	public:
		using Node_t = SingleLinkedNode<Data_t>;
	private:
		mutable std::mutex Tail_mtx;
		mutable std::mutex Head_mtx;
		std::condition_variable ConditionVar;

		std::unique_ptr<Node_t> Head;
		Node_t* Tail;
		std::atomic<size_t> Size;//size operations are non blocking

		std::unique_ptr<Node_t> CreateDummyNode()
		{
			return std::make_unique<Node_t>(Node_t{ Data_t{} });
		}
		
		//pops the head without bloacking, should be called inside the lock region
		Data_t pop_head_unsafe();
	public:

		LB_FineGrainedQueue() : Head{ CreateDummyNode() }
		{
			Tail = Head.get();//call here since we do not know the order of call in the initializer list
		}
		//TODO: allow deep copy
		LB_FineGrainedQueue(const LB_FineGrainedQueue&) = delete;
		LB_FineGrainedQueue& operator=(const LB_FineGrainedQueue&) = delete;

		NODISCARD inline size_t size() const override
		{
			return Size.load(std::memory_order::memory_order_acquire);
		};
		NODISCARD inline bool empty()const override
		{
			return size() == 0;
		};

		void push(const Data_t& data) override
		{
			push(Data_t{ data });//call the move overloaded version of the push function
		}
		void push(Data_t&& data)noexcept override
		{
			std::unique_ptr<Node_t> newDummy{ CreateDummyNode() };
			Node_t*const newTail{ newDummy.get() };

			//lock region
			{
				std::lock_guard Lock{ Tail_mtx };

				Tail->Data = std::move(data);
				Tail->NextNode = std::move(newDummy);
				Tail = newTail;
			}
			Size.fetch_add(1, std::memory_order::memory_order_acq_rel);
			ConditionVar.notify_one();
		}
		 
		NODISCARD Data_t pop() override
		{
			std::unique_lock Lock{ Head_mtx };
			ConditionVar.wait(Lock, [&] {return !empty(); });
			return pop_head_unsafe();
		}
		NODISCARD std::optional<Data_t> try_pop() noexcept
		{
			//lock before checking size, to avoid invalid data
			std::lock_guard Lock{ Head_mtx };
			if (empty())
			{
				return std::nullopt;
			}
			else
			{
				return pop_head_unsafe();
			}
			
		}
	};
	
	template<typename Data_t>
	inline Data_t LB_FineGrainedQueue<Data_t>::pop_head_unsafe()
	{
		Data_t res{ std::move(Head->Data) };
		Head = std::move(Head->NextNode);
		Size.fetch_sub(1, std::memory_order::memory_order_acq_rel);//must hapen inside the lock, to keep the not empty condiyion for other threads
		return res;
	}

}