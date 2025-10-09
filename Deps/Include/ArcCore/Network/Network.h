#pragma once
#include"NetworkTypes.h"
#include<optional>
/*
* low level interfacing imple for socket APIs
**
*/
namespace Network
{
	void InitSocketAPI();

	void CloseSocketAPI();

	//trys to get a socket, optional since the operation might fail
	std::optional<NativeSocket_t> MakeSocket(EAddressFamily AF, ESocketType type, int flags);
	std::optional<NativeSocket_t> MakeSocket(int AF, int type, int flags);

	void CloseSocket(NativeSocket_t sock);

	bool IsValidSocket(NativeSocket_t sock) noexcept;

	
}