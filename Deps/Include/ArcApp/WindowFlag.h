#pragma once
namespace ARC::App
{
	enum WindowFlags {
		INVISIBLE = 0B00'00'00'01,
		FULL_SCREEN = 0B00'00'00'10,
		BORDERLESS = 0B00'00'01'00,
		DOUBLE_BUFFERED = 0B00'00'10'00,
		V_SYNC = 0B00'01'00'00,
	};
}


