#include "Input.h"

namespace AppComponents::InputManagement
{
	//static members initialization
	std::vector<Input::Dispatcher_t> Input::InputDispatchers{};
	std::mutex Input::Input_mtx;

	inline void Input::Register(Dispatcher_t dispatcher)
	{
		std::lock_guard Lock{ Input_mtx };
		InputDispatchers.push_back(dispatcher);
	}

	//dispatchs registered inputs
	inline void Input::DispatchInputs()
	{
		std::lock_guard Lock{ Input_mtx };
		for (auto& dispatcher : InputDispatchers)
		{
			dispatcher();
		}
	}
}

