#pragma once
#include"Graphics/Image_View.h"
#include<map>
namespace Graphics
{
	class SimpleTexture
	{
		unsigned TexID{};
		int Height{};
		int Width{};
		EColorFormat ColorFormat = EColorFormat::RGBA;

	public:
		SimpleTexture() = default;
		SimpleTexture(unsigned ResourceID, EColorFormat format = EColorFormat::RGBA, void* moduleHandle = nullptr);
		SimpleTexture(std::string_view TextureSrcPath, EColorFormat format = EColorFormat::RGBA, bool autoUnloadImg = true);
		SimpleTexture(Graphics::Image_View img);

		SimpleTexture(SimpleTexture&& other)noexcept;
		SimpleTexture& operator=(SimpleTexture&& other)noexcept;

		SimpleTexture(const SimpleTexture&) = delete;
		SimpleTexture& operator=(const SimpleTexture&) = delete;


		~SimpleTexture();

		bool Create(unsigned ResourceID, EColorFormat format = EColorFormat::RGBA, void* moduleHandle = nullptr);
		bool Create(std::string_view TextureSrcPath, EColorFormat format = EColorFormat::RGBA, bool autoUnloadImg = true);
		bool Create(Graphics::Image_View img);

		void Reset();

		inline unsigned GetTexID()const { return TexID; }

		inline int GetWidth()const { return Width; }
		inline int GetHeight()const { return Width; }
		inline EColorFormat GetColorFormat()const { return ColorFormat; }
		inline bool IsValid()const { return TexID != 0; }
	};

	//this class alows us to avoid creating duplicate textures
	class ResourceTexManager
	{
		static std::map<std::string, std::weak_ptr<SimpleTexture>> ManagedTex;
	public:
		static std::shared_ptr<SimpleTexture> GetOrCreate(unsigned ResourceID, EColorFormat format = EColorFormat::RGBA, void* moduleHandle = nullptr);
		static std::shared_ptr<SimpleTexture> GetOrCreate(std::string path, EColorFormat format = EColorFormat::RGBA);
		static size_t CleanExpiredRefs();
	};
}

