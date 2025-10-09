#pragma once
#include"Macros.h"
#include<memory>
#include"Graphics/ColorFormat.h"
#include"Concurrency/LB_Map.h"
#include<filesystem>

namespace Graphics
{
	class Image;
	class Image_View;
}
namespace ResourceManagement
{
	/*
	*a thread safe image manager static class
	*allow loading , retriving and refrence safe destruction of images
	*/
	class  ARCRX_API ImageManager
	{
	public:
		//path type
		using Path_t = typename std::filesystem::path;
		//optional EColorFormat
		using OptColFormat = typename std::optional<Graphics::EColorFormat>;

		//retrives a preloaded image from the map, return invalid if the image was not found, Get image strong if you want the image to be loaded instead
		static  Graphics::Image_View GetImage(const Path_t& ImgPath);

		/* trys to find the image in the map, if not found will attempt to loaded from device
		*will fail only if path is not valid of could not load image
		*@requiredFormat: needed for image if it was not found , do not set to get file default
		*/
		static  Graphics::Image_View GetImage_Strong(const Path_t& ImgPath, OptColFormat&& requiredFormat);
		
		/*
		*load image only if it not found in the map
		*@ImgPath : image path
		*@requiredFormat : optional param for color format , do not set if you want source file format
		*/
		static bool LoadImage_Unique(const Path_t& ImgPath, OptColFormat&& requiredFormat);

		//loaded an image without path safety or override checking, returns false if the loading process failed
		static bool LoadImage_Unchecked(const Path_t& ImgPath, OptColFormat&& requiredFormat);

		//trys to remove images reference from the map thus allowing it to be destroyed [ only if the map is the only referenceing agent]
		static bool TryUnLoadImage(const Path_t& ImgPath);

		friend class ::Graphics::Image;//necessary for the image class to call freeimage in the destructor
	private:
		//writes the given image from file, does not add it to ,map
		static bool LoadImage_Unchecked(Graphics::Image& Img, const Path_t& ImgPath, OptColFormat&& requiredFormat);
		
		//deallocates the loaded image data, without updating map or checking references, do not call directly: image destructor will call us
		static void FreeImage(Graphics::Image& img);

		//mapping image path with image*
		static Concurrency::LB_MAP<const Path_t, std::shared_ptr<Graphics::Image>, std::hash<std::filesystem::path>> LoadedImages_Map;
	};
}
