#pragma once
#include"Core/ArcCore.h"
#include<memory>
namespace Concurrency {
/*
* a data holder node with a pointer to the next node
* move only construction is supported duw to the use of unique pointers
*/
template<typename T>
class NODISCARD ARC_API  SingleLinkedNode
{
	static_assert(!std::is_pointer_v<T>, "SingleLinkedNode class was not designed to manage raw pointer data types: will not own the data!");
	static_assert(std::is_move_constructible_v<T>&& std::is_move_assignable_v<T>, "SingleLinkedNode Expectes a movable parameter!");

public:
	//type of the data held by the node

	using type = T;
	SingleLinkedNode() = delete;
	SingleLinkedNode(const SingleLinkedNode& other) = delete;
	SingleLinkedNode& operator=(const SingleLinkedNode& other) = delete;

	SingleLinkedNode(SingleLinkedNode&& other)noexcept = default;
	SingleLinkedNode& operator=(SingleLinkedNode&& other)noexcept = default;

	SingleLinkedNode(const T& data, std::unique_ptr<SingleLinkedNode<T>>&& nextNode = nullptr) :Data{ data }, NextNode{ std::move(nextNode) } {};
	SingleLinkedNode(T&& data, std::unique_ptr<SingleLinkedNode<T>>&& nextNode = nullptr)noexcept :Data{ std::move(data) }, NextNode{ std::move(nextNode) }{};

	T Data;
	std::unique_ptr<SingleLinkedNode<T>> NextNode;
};

}