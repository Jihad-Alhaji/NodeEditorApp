#pragma once
#include"Structures/Delegate.h"

struct FArcAppContext
{
	void* AppContext{nullptr};

	void* GUI_Context{ nullptr };
	void* GUI_Alloc{ nullptr };
	void* GUI_Free{ nullptr };
	void* GUI_User_Data{ nullptr };


	static  FArcAppContext* Context;
	static void SetArcAppContext(FArcAppContext* ctx);
};

