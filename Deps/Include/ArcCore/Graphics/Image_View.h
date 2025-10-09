#pragma once
#include"Core/ArcCore.h"
#include"ColorFormat.h"
#include"Image.h"
#include<memory>

namespace Graphics
{
	class NODISCARD ARC_API Image_View
	{
		std::shared_ptr<const Image> _ImagePtr;
	public:
		using byte_t = unsigned char;

		Image_View() = default;
		explicit Image_View(std::shared_ptr<const Image>&& image);
		explicit Image_View(const std::shared_ptr<const Image>& image);
		Image_View(const Image_View&) = default;
		Image_View(Image_View&&)noexcept = default;
		Image_View& operator=(const Image_View&) = default;
		Image_View& operator=(Image_View&&)noexcept = default;
		//TODO: add image view functionality here
		//read point of a channel
		// read a pixel
		// compile time pixel type
		//get a channel iterator
		//get a pixel iterator
		//all components must be light weight

		bool IsValid()const noexcept { return _ImagePtr != nullptr; }
		int GetWidth()const noexcept { return _ImagePtr->Width; }
		int GetHeight()const noexcept { return _ImagePtr->Height; }
		EColorFormat GetSourceFormate()const noexcept { return _ImagePtr->srcFormat; }
		EColorFormat GetFormate()const noexcept { return _ImagePtr->Format; }
		std::wstring_view GetPath()const noexcept { return _ImagePtr->FilePath; }
		const byte_t* GetData()const noexcept { return _ImagePtr->Data; }
		//remove the reference to the inner object
		void Reset()noexcept { _ImagePtr.reset(); }
	};
}

