#pragma once
namespace Network
{
	using NativeSocket_t = unsigned long long;
	using Port_t = unsigned short;
	using IP_t = unsigned long;

	enum class EAddressFamily: int
	{
		INET_4 = 2,
		INET_6
	};

	enum class ESocketType : int
	{
		STREAM = 1
	};
}