#pragma once
#include"Core/ArcCore.h"
#include"Structures/Delegate.h"
struct GLFWwindow;
namespace ARC::App
{
	class NODISCARD ARC_API Window
	{
	public:
		Window();
		~Window();

		int GetScreenWidth() const { return SCR_WIDTH; }
		int GetScreenHeight() const { return SCR_HEIGHT; }
		int GetScreenPos_X()const { return LocX; }
		int GetScreenPos_Y()const { return LocY; }

		int CreateWindow(int width, int height, const char* title, unsigned int winFlags);

		bool ShouldClose() const;

		bool IsMaximized()const;
		bool IsMinimized()const;

		//notifiyes the underlying framework that we should close
		void Close()noexcept;

		void SwapBuffers();

		//openGl buffer bitmasks : used for clear() ... [not protected]
		void SetBufferBits(unsigned int bits);

		void Clear();

		GLFWwindow* GetWinObject() { return _Win; }

		//chacks the state of the window
		//dispatched events
		void Update();
		//delegates position in screen space [int x, int y]
		Structures::TMultiDelegate<int, int> OnWindowLocationChanged;

		//delegates size in screen space [int w, int h]
		Structures::TMultiDelegate<int, int> OnWindowSizeChanged;
	protected:
		GLFWwindow* _Win;
		int SCR_WIDTH, SCR_HEIGHT;
		int LocX, LocY;
		int BufferBits;
	};

}


