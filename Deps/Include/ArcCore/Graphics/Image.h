#pragma once
#include"core/ArcCore.h"
#include"Graphics/ColorFormat.h"
#include<string>
namespace ResourceManagement
{
	class ImageManager;
}
namespace Graphics
{
	/*
	*a RAII class representing a IO file image
	*generallay used in tandem with resouece manager
	*/
	class NODISCARD ARC_API Image
	{
	protected:
		 int Width;
		 int Height;
		 EColorFormat srcFormat;// the original image file channels, use channels for iteration instead
		 EColorFormat Format;// the actual channels of the data stored in this image class
		
		 unsigned char* Data;
		 std::wstring FilePath;

		
		friend class Image_View;//used for safe access to the image
		friend class ::ResourceManagement::ImageManager;//manages the image, keeps reference counts and allocates/deallocates the resource
		
	public:
		Image();
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		Image(Image&& other)noexcept;
		Image& operator=(Image&& other)noexcept;
		
		Image(int w, int h, EColorFormat srcForma, EColorFormat Forma , std::wstring_view path , unsigned char* dat);
		
		void swap(Image& Second)noexcept;
		~Image();//calls the manager and frees up the image
	};
	
}

