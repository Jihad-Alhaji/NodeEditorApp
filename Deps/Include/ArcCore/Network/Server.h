#pragma once
#include"Core/ArcCore.h"
#include<string>
#include"NetworkTypes.h"
#include"Structures/Delegate.h"
#include<unordered_map>
#include"Concurrency/LockFreeQueue.h"
namespace Network
{
	class NODISCARD ARC_API FServer
	{
		NativeSocket_t ListeningSocket;
		//ip to socket map
		std::unordered_map<std::string, NativeSocket_t> Clients;
		char* Buffer;
		const int Size;
		bool bConnected;
		const Port_t Port;

		//clients that are pending to be added to the server
		Concurrency::LFQueue<std::pair<std::string, NativeSocket_t>> PendingClients;
	public:
		FServer(Port_t port, int buffersize);
		~FServer();

		inline Port_t GetPort()const { return Port; }
		bool IsConnected()const;
		//disconnects the server from all clients
		void DisConnect();
		void DisConnectClient(std::string ClientIP);
		bool WaitForConnection();//thread safe
		void AddPendingClients();
		void Recieve();

		Structures::TMultiDelegate<bool> OnConnectionStatusChanged;
		Structures::TMultiDelegate<std::string /*ip*/,bool> OnClientConnectionChanged;
		Structures::TMultiDelegate<std::string/*IP*/, std::string/*Data Buffer*/> OnDataRecieved;
	};

}

