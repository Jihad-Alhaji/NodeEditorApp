#pragma once
#include"InputEnums.h"
#include"Macros.h"
namespace AppComponents {
namespace InputManagement
{
	/*
	* a platform independent key enquirer class
	*/
	class  ARCINPUT_API KeyEnquirer
	{
	public:
		static EKeyAcions GetKeyState(EKeys key)noexcept;
		static long GetMousePos_X()noexcept;
		static long GetMousePos_Y()noexcept;
	};
}

}
