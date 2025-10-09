#pragma once
#include"MessageQueue.h"
#include"Core/ArcCore.h"
namespace Messaging {
	class Sender;
	class MessageBase;
	class Dispatcher;

	//type alias used in this name space message queues
	
	
	class NODISCARD ARC_API Reciever
	{	
	public:
		///move only
		Reciever() = default;
		Reciever(const Reciever&) = delete;
		Reciever& operator=(const Reciever&) = delete;

		//creates a sender that referece the internal message queue
		NODISCARD Sender CreateSender();

		Dispatcher wait();
	private:
		MsgQueue_t MsgQueue;//reciever owns the queue
	};
}


