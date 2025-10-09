#pragma once
#include"Concurrency/LB_SequentialContainer.h"
#include"MessageBase.h"
#include<queue>
namespace Messaging
{

	//you can change queue element type and the queue it self from here
	//changes will propagate to the entire namespace

	using QueueElement_t = std::shared_ptr<MessageBase>;//messaging api requires a smart pointer interface
	using MsgQueue_t = Concurrency::LB_SequentialContainer<QueueElement_t, std::queue<QueueElement_t>, &std::queue<QueueElement_t>::front>;
	//using MsgQueue_t = Concurrency::LB_FineGrainedQueue<QueueElement_t>;
	//TODO: using fine grained queue can causes stack overflow error , the built in queue does not??
}