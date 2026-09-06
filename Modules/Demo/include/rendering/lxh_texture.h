#pragma once
#include <lxh_device.h>

#include <string>
#include <vector>

namespace lxh
{
	class Texture2D
	{
	public:
		// Loads an RGBA8 image from disk. Pass VK_FORMAT_R8G8B8A8_SRGB for color
		// (albedo) maps and VK_FORMAT_R8G8B8A8_UNORM for data (normal/roughness/AO) maps.
		Texture2D(
			LxhDevice& device,
			const std::string& filepath,
			VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
			uint32_t mipLevels = 1);

		// Creates a texture from raw RGBA8 pixels (used for 1x1 fallback textures).
		Texture2D(
			LxhDevice& device,
			const std::vector<unsigned char>& pixels,
			VkExtent3D extent,
			VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
			uint32_t mipLevels = 1);

		~Texture2D();
		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;

		VkImageView getImageView() const { return m_imageView; }
		VkSampler getSampler() const { return m_sampler; }
		VkImage getImage() const { return m_image; }

		VkFormat getFormat() const { return m_format; }
		VkExtent3D getExtent() const { return m_extent; }
		uint32_t getMipLevels() const { return m_mipLevels; }

		const VkDescriptorImageInfo& getDescriptorInfo() const { return m_descriptor; }

	private:
		void createFromPixels(const unsigned char* pixels);
		void createImageView();
		void createSampler();

		static uint32_t bytesPerFormat(VkFormat format);

		LxhDevice& m_device;
		VkImage m_image = VK_NULL_HANDLE;
		VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;
		VkSampler m_sampler = VK_NULL_HANDLE;

		VkDescriptorImageInfo m_descriptor{};

		VkFormat m_format;
		VkExtent3D m_extent{};
		uint32_t m_mipLevels;
	};
}
