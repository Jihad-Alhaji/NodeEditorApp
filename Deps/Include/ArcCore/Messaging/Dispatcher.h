#pragma once
#include"Core/ArcCore.h"
#include"MessageQueue.h"
#include"Message.h"
#include<functional>
#include<type_traits>

namespace Messaging {
	//forward declaration for dispatcher
	class Dispatcher;

	template<typename prevDispatcher, typename msgContent, typename func_t = void()>
	class TemplateDispatcher
	{
		MsgQueue_t* MsgQueue;
		prevDispatcher* PrevDispatcher;
		std::function<func_t> Func;
		bool bChained;

		//make all instance of this template friends
		template<typename prevDispatcher_n, typename msgContent_n, typename func_t_n>
		friend class TemplateDispatcher;

		void wait_and_dispatch();
		bool dispatch(QueueElement_t&& msg);

	public:

		TemplateDispatcher(TemplateDispatcher&& other)noexcept;
		TemplateDispatcher(MsgQueue_t* msgQueue, prevDispatcher* prev, std::function<func_t>&& func);

		~TemplateDispatcher() noexcept(false);

		//customly handle a message
		template< typename otherMsgContent, typename otherFunc_t = void()>
		TemplateDispatcher<TemplateDispatcher, otherMsgContent, otherFunc_t> handle(std::function<otherFunc_t>&& func);
	};

	template<typename prevDispatcher, typename msgContent, typename func_t>
	inline TemplateDispatcher<prevDispatcher, msgContent, func_t>::TemplateDispatcher(TemplateDispatcher&& other) noexcept :
		PrevDispatcher{ other.PrevDispatcher }, MsgQueue{ other.MsgQueue }, Func{ std::move(other.Func) }, bChained{ other.bChained }
	{
		other.bChained = true;
		//TODO: assert for dispatcher classes only
	}

	template<typename prevDispatcher, typename msgContent, typename func_t>
	inline TemplateDispatcher<prevDispatcher, msgContent, func_t>::TemplateDispatcher(MsgQueue_t* msgQueue, prevDispatcher* prev, std::function<func_t>&& func) :
		MsgQueue{ msgQueue }, PrevDispatcher{ prev }, Func{ std::move(func) }, bChained{ false }
	{
		prev->bChained = true;
	}

	template<typename prevDispatcher, typename msgContent, typename func_t>
	inline TemplateDispatcher<prevDispatcher, msgContent, func_t>::~TemplateDispatcher() noexcept(false)
	{
		if (!bChained)
			wait_and_dispatch();
	}

	template<typename prevDispatcher, typename msgContent, typename func_t>
	inline void TemplateDispatcher<prevDispatcher, msgContent, func_t>::wait_and_dispatch()
	{
		for (;;)
		{
			if (dispatch(MsgQueue->pop()))
				break;
		}
	}

	template<typename prevDispatcher, typename msgContent, typename func_t>
	inline bool TemplateDispatcher<prevDispatcher, msgContent, func_t>::dispatch(QueueElement_t&& msg)
	{
		if (Message<msgContent> * MSG{ dynamic_cast<Message<msgContent>*>(msg.get()) })
		{
				if constexpr (std::is_same_v<func_t, void()>)
					Func();
				else
				Func(std::move(MSG->Content));

			return true;
		}
		return PrevDispatcher->dispatch(std::move(msg));
	}


	template<typename prevDispatcher, typename msgContent, typename func_t>
	template<typename otherMsgContent, typename otherFunc_t>
	inline TemplateDispatcher<TemplateDispatcher<prevDispatcher, msgContent,func_t>, otherMsgContent,otherFunc_t> TemplateDispatcher<prevDispatcher, msgContent,func_t>::handle(std::function<otherFunc_t>&& func)
	{
		return TemplateDispatcher<TemplateDispatcher, otherMsgContent,otherFunc_t>{MsgQueue, this, std::move(func)};
	}
}
namespace Messaging {
	class NODISCARD ARC_API Dispatcher
	{

		//allow template prevDispatcher to access privates :|
		template<typename prevDispatcher, typename msgContent, typename func_t>
		friend class TemplateDispatcher;

	public:
		//creates an unchaned prevDispatcher
		explicit Dispatcher(MsgQueue_t* msgQueue);

		//chain other to a new unchaines dispatcher
		Dispatcher(Dispatcher&& other) noexcept;

		//the prevDispatcher action occures in the destructor
		~Dispatcher() noexcept(false);

		//customly handle a message
		template< typename msgContent, typename func_t = void()>
		TemplateDispatcher<Dispatcher, msgContent,func_t> handle(std::function<func_t>&& func);



	private:
		MsgQueue_t* MsgQueue;
		bool bChained;

		//waits for a message, checks for closure to throw else and return false to indicate that it did not handle
		void wait_and_dispatch();
		bool dispatch(QueueElement_t&& msg);

		void swap(Dispatcher& other)noexcept;
	};

	template<typename msgContent, typename func_t>
	inline TemplateDispatcher<Dispatcher, msgContent, func_t> Dispatcher::handle(std::function<func_t>&& func)
	{
		return TemplateDispatcher<Dispatcher, msgContent, func_t>{MsgQueue, this, std::move(func)};
	}
}