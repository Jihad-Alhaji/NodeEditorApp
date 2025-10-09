#pragma once
#include"Core/ArcCore.h"
namespace Messaging {
	//base class for messages to allow polymorphism
	class NODISCARD ARC_API MessageBase
	{
	public:
		virtual ~MessageBase() = default;
	};
}


