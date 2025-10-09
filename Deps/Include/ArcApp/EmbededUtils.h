#pragma once
#include<Graphics/Image_View.h>
//loads an embeded png source asset
std::shared_ptr<Graphics::Image> LoadPngFromResource(unsigned int resourceID, void* moduleHandle = nullptr);


