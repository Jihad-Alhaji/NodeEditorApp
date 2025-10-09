#pragma once
#include "MessageBase.h"
#include<type_traits>

namespace Messaging {

	template<class MsgType>
	class  NODISCARD Message : public MessageBase
	{
	public:
		//data of the message
		MsgType Content;

		Message(const Message& other);
		Message(Message&& other)noexcept;

		explicit Message(const MsgType& content);
		explicit Message(MsgType&& content)noexcept;

		Message& operator=(const Message& other);
		Message& operator=(Message&& other)noexcept;

		

		~Message() = default;
	};

	template<class MsgType>
	inline Message<MsgType>::Message(const Message& other) :Message{ other.Content }
	{
	}

	template<class MsgType>
	inline Message<MsgType>::Message(Message&& other) noexcept :Message{ std::move(other.Content) }
	{
	}

	template<class MsgType>
	inline Message<MsgType>& Message<MsgType>::operator=(const Message& other)
	{
		Message inter{ other };
		std::swap(Content, inter.Content);
		return *this;
	}

	template<class MsgType>
	inline Message<MsgType>& Message<MsgType>::operator=(Message&& other) noexcept
	{
		Message inter{ std::move(other) };
		std::swap(Content, inter.Content);
		return *this;
	}

	template<class MsgType>
	inline Message<MsgType>::Message(const MsgType& content) :Content{ content }
	{
		static_assert(std::is_copy_constructible_v<MsgType> || std::is_move_constructible_v<MsgType>, "Message:: MsgType must be copy and/or move constructable");
	}
	template<class MsgType>
	inline Message<MsgType>::Message(MsgType&& content)noexcept :Content{ std::move(content) }
	{
	}
}

