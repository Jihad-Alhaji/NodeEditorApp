#pragma once

#ifdef ARCLOG_EXPORT
#define ARCLOG_API __declspec(dllexport)
#else
#define ARCLOG_API __declspec(dllimport)
#endif	