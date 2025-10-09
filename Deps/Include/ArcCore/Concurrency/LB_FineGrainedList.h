#pragma once
#include"Core/ArcCore.h"
#include<mutex>
#include<memory>
#include<optional>
#include<functional>

namespace Concurrency 
{
	template<typename value_t>
	class NODISCARD ARC_API LB_FineGrainedList
	{
		//internal node structure
		struct Node
		{
			mutable std::mutex mtx;
			std::unique_ptr<Node> NextNode;
			std::shared_ptr<value_t> Data;
			Node() = default;
			Node(value_t&& val):Data{std::make_shared<value_t>(std::move(val))}
			{}
		};
		Node Head{};
	public:
		using value_type = value_t;
		using EntryInspector_t = std::function<void(const value_t& val)>;
		using EntryModifier_t = std::function<void(value_t& val)>;
		using Predicate_t = std::function<bool(const value_t& val)>;

		LB_FineGrainedList() = default;
		//TODO: allow deep copy
		LB_FineGrainedList(const LB_FineGrainedList&) = delete;
		LB_FineGrainedList& operator=(const LB_FineGrainedList&) = delete;

		void push_front(value_t&& value)noexcept
		{
			std::unique_ptr<Node> new_Node{ new Node{std::move(value)} };
			std::lock_guard<std::mutex> Lock{ Head.mtx };
			new_Node->NextNode = std::move(Head.NextNode);
			Head.NextNode = std::move(new_Node);
		}
		void push_front(const value_t& value)noexcept
		{
			push_front(value_t{ value });
		}
		
		void foreach(EntryInspector_t&& inspector)const
		{
			const Node* current{ &Head };
			std::unique_lock<std::mutex> LockCurrent{ current->mtx };
			while (const Node* const next{ current->NextNode.get() })
			{
				std::unique_lock<std::mutex> LockNext{ next->mtx };
				LockCurrent.unlock();
				inspector(*next->Data);
				current = next;
				LockCurrent = std::move(LockNext);
			}
		}
		void foreach(EntryModifier_t&& modifier)
		{
			Node* current{ &Head };
			std::unique_lock<std::mutex> LockCurrent{ current->mtx };
			while (Node*const next{ current->NextNode.get() })
			{
				std::unique_lock<std::mutex> LockNext{ next->mtx };
				LockCurrent.unlock();
				modifier(*next->Data);
				current = next;
				LockCurrent = std::move(LockNext);
			}
		}
		std::optional<value_t> find_first_if(Predicate_t&& predicate)
		{
			const Node* current{ &Head };
			std::unique_lock<std::mutex> LockCurrent{ current->mtx };
			while (const Node* const next{ current->NextNode.get() })
			{
				std::unique_lock<std::mutex> LockNext{ next->mtx };
				LockCurrent.unlock();
				if (predicate(*next->Data))
				{
					return *next->Data;
				}
				current = next;
				LockCurrent = std::move(LockNext);
			}
			return std::nullopt;
		}
		//removed items if the condition holds , returns number of removed items
		size_t remove_if(Predicate_t&& predicate)
		{
			const Node* current{ &Head };
			size_t removed{};
			std::unique_lock<std::mutex> LockCurrent{ current->mtx };
			while (const Node* const next{ current->NextNode.get() })
			{
				std::unique_lock<std::mutex> LockNext{ next->mtx };
				if (predicate(*next->Data))
				{
					auto nodeToRemove{ std::move(current->NextNode) };
					current->NextNode = std::move(next->NextNode);
					LockCurrent.unlock();
					++removed;
				}
				else
				{
					LockCurrent.unlock();
					current = next;
					LockCurrent = std::move(LockNext);
				}
			}
			return removed;
		}
	};
}