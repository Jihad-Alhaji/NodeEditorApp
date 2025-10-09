#pragma once
#include<functional>
#include<queue>

namespace ARC
{
	class Event
	{
	public:
		Event(std::function<void()>&& e, double  delay = 0.0);
		Event(const std::function<void()>& e, double  delay = 0.0);
		Event(const Event&) = default;
		Event(Event&&) = default;
		Event& operator=(const Event&) = default;
		Event& operator=(Event&&) = default;

		void operator()(void);
	
		//the amount of time to delay this event
		double m_Delay;
	protected:
		//the functor to call when executing this event
		std::function<void()> m_Event;
	};
	class EventDispatcher
	{
	protected:
		std::queue<Event> m_EventQueue;
	public:
		bool PushEvent(Event e);
		void DispatchEvents(double deltatime);
	};
}


