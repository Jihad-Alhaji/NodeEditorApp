#pragma once
#include"MessageQueue.h"
#include"Message.h"
#include"Core/ArcCore.h"
#include<memory>
namespace Messaging {
	
	class NODISCARD ARC_API Sender
	{

	public:
		Sender();
		explicit Sender(MsgQueue_t* msgQueue);

		Sender(const Sender& other);
		Sender& operator=(const Sender& other);
		
		Sender(Sender&& other)noexcept;
		Sender& operator=(Sender&& other)noexcept;

//sends a msg if we can send
		template<typename MsgContent>
			void Send(const MsgContent& msgContent);

		template<typename MsgContent>
			void Send(MsgContent&& msgContent);

			//true if a queue is referenced
		bool CanSend() const noexcept { return MsgQueue; }

		void swap(Sender& other) noexcept;
	private:
		MsgQueue_t* MsgQueue;
	};
	
	template<typename MsgContent>
	inline void Sender::Send(const MsgContent& msgContent)
	{
		if(MsgQueue)
			MsgQueue->push(std::make_shared<Message<MsgContent>>(msgContent));
	}
	template<typename MsgContent>
	inline void Sender::Send(MsgContent&& msgContent)
	{
		if (MsgQueue)
			MsgQueue->push(std::make_shared<Message<MsgContent>>(std::move(msgContent)));
	}
}


