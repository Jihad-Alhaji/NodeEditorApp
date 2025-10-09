#pragma once
#include<atomic>
#include"Macros.h"
namespace AppComponents {
namespace InputManagement
{
	/*
	*a class manager for inputs event dispaters
	*thread safe: Atomic
	*/
	class  ARCINPUT_API InputManager
	{
		//used to stop the manager and prevent multiple threades from calling run
		static std::atomic<bool> bRunState;
		//min milliseconds between dispatch calls
		inline const static double MinDeltaTick{5};
	public:
		//loops requesting dispatches from key inputs till stop is called
		static void Run();
		
		//sends close_queue
		static void Stop();

		static bool IsRunning() noexcept;
	};


}
}