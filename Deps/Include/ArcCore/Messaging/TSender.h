#pragma once
#include"Sender.h"
namespace Messaging {

/* a sernder class that can only send messages of specific type */
template<typename Msg_t>
class NODISCARD TSender: protected Sender
{

public:
	explicit TSender(MsgQueue_t* msgQueue) : Sender{msgQueue} {};
	explicit TSender(Sender&& other)noexcept : Sender{std::move(other)}{};
	explicit TSender(const Sender& other)noexcept : Sender{other}{};

	using Sender::CanSend;
	using Sender::swap;

	void Send(const Msg_t& msgContent);
	void Send(Msg_t&& msgContent);
	//insert operators for facilitating usage
	TSender<Msg_t>& operator<<(Msg_t& msgContent);
	TSender<Msg_t>& operator<<(Msg_t&& msgContent);
};

template<typename Msg_t>
inline void TSender<Msg_t>::Send(const Msg_t& msgContent)
{
	Sender::Send<Msg_t>(msgContent);
}

template<typename Msg_t>
inline void TSender<Msg_t>::Send(Msg_t&& msgContent)
{
	Sender::Send<Msg_t>(std::move(msgContent));
}
template<typename Msg_t>
inline TSender<Msg_t>& TSender<Msg_t>::operator<<(Msg_t& msgContent)
{
	Send(msgContent);
	return *this;
}
template<typename Msg_t>
inline TSender<Msg_t>& TSender<Msg_t>::operator<<(Msg_t&& msgContent)
{
	Send(std::move(msgContent));
	return *this;
}
}

