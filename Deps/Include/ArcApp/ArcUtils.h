#pragma once
#include<string>
#include<filesystem>
#include"imgui.h"
namespace ARC
{
	static const char* ThemeFileExtension = ".jat";

	//exports the current theme to a theme file in the resources directory
	void SaveArcTheme(std::string themename);

	void LoadArcTheme(std::string themename);

	std::vector<std::filesystem::path> GetAllFilesInDirectory(std::string dir, std::string extensionfilter);
}
