#pragma once
#include"Macros.h"
#include<string>
#include<Concurrency/LockFreeQueue.h>

namespace Logging
{
	enum class ARCLOG_API LogVerbosity
	{
		NOTE,
		WARNING,
		ARCERROR
	};

	//takes a verbosity and return a braket surrounded label for it
	const char* GetVerbosityLabel(LogVerbosity verb)noexcept;

	using LogQueue_t = typename Concurrency::LFQueue<std::string>;

	class ARCLOG_API Logger
	{
		explicit Logger(std::string&& Name, LogQueue_t* msgQueue);
		friend class LogSystem;
	public:
		Logger(const Logger&) = default;
		Logger& operator=(const Logger&) = default;

		Logger(Logger&& other)noexcept;
		Logger& operator=(Logger&& other)noexcept;

		void Send(LogVerbosity verbosity, std::string msg);

		//true if a queue is referenced
		inline bool CanSend() const noexcept { return MsgQueue != nullptr; }

		void swap(Logger& other) noexcept;

	private:
		LogQueue_t* MsgQueue;
		std::string LoggerName;
	};
}


