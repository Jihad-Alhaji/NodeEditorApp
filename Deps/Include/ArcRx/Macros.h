#pragma once

#ifdef ARCRX_EXPORT
#define ARCRX_API __declspec(dllexport)
#else
#define ARCRX_API __declspec(dllimport)
#endif 
