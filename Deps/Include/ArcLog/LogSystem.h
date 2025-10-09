#pragma once
#include"Logger.h"
#include"Macros.h"
#include<memory>
namespace std
{
	using ostream = basic_ostream<char, char_traits<char>>;
}

namespace Logging
{
	//a central async log system
	//allows for creation on cusotm loggers
	//thread safe
	class ARCLOG_API LogSystem
	{
		class LogSystem_impl* LogSystem_Internal;
		
	public:
		explicit LogSystem(std::ostream* ostream);//creates an adaptor log system to an output stream
		explicit LogSystem(const char* fileName);//creates a log system to a file
		~LogSystem();
		//returns a sender that cal log into this logSystem
		Logger CreateLogger(std::string name);

		//logs all log queue , if bOnce then it will exit otherwide will loop indefinitly
		void Run(bool bOnce = false)noexcept;

		void Stop()noexcept;

		bool IsRunning()const noexcept;

		static LogSystem ArcLogSys;//global log system
		static Logger ArcLogger;//global logger
	};

	
}
#define CustomLog(logger, LogVerb, msg) logger.Send(Logging::LogVerbosity::##LogVerb , msg)

#define ArcLog(LogVerb, msg) CustomLog(Logging::LogSystem::ArcLogger, LogVerb,msg);
#define ArcNote(msg) ArcLog(NOTE,msg)
#define ArcWarning(msg) ArcLog(WARNING,msg)
#define ArcError(msg) ArcLog(ARCERROR,msg)