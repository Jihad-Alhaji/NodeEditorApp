#pragma once
#include"Structures/Delegate.h"
#include"NetworkTypes.h"
#include<string>
namespace Network
{
	class FClient
	{
		bool bConnected{false};
		NativeSocket_t RecieverSocket{};
		Port_t Port;
		std::string IP_Address;
	public:
		void Send(std::string_view msg);
		void DisConnect();
		bool Connect(std::string_view ip, Port_t port);
		bool IsConnected()const { return bConnected; };
		~FClient();

		Structures::TMultiDelegate<bool> OnConnectionStatusChanged;
	};
}


