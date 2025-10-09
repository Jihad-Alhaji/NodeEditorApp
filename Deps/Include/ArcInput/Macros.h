#pragma once

#ifdef ARCINPUT_EXPORT
#define ARCINPUT_API __declspec(dllexport)
#else
#define ARCINPUT_API __declspec(dllimport)
#endif 