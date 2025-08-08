#pragma once
#include <lxh_device.h>
#include <lxh_buffer.h>
#include <vector>
#include<memory>
namespace lxh
{
	class Texture2D
	{
	public:
		Texture2D(
			const std::string& filepath,
			VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
			uint32_t mipLevels = 1);

		Texture2D(
			VkFormat format,
			VkExtent3D extent,
			uint32_t mipLevels);

		~Texture2D();


		VkImageView getImageView() const { return m_imageView; }
		VkSampler getSampler() const { return m_sampler; }
		VkImage getImage() const { return m_image; }
		VkDeviceMemory getImageMemory() const { return m_imageMemory; }

		VkFormat getFormat() const { return m_format; }
		VkExtent3D getExtent() const { return m_extent; }
		uint32_t getMipLevels() const { return m_mipLevels; };

		VkDescriptorImageInfo& GetDescriptorRef()
		{
			return m_Descriptor;
		}

		void UpdateDescriptor();

	private:
		void createTexture2DImage(const char* filename);

		bool createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t layerCount = 1, uint32_t mipLevels = 1);
		bool createTexture2DSampler(VkSampler* sampler, VkFilter minFilter = VK_FILTER_LINEAR, VkFilter maxFilter = VK_FILTER_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

		void createImageBuffer(VkDeviceSize  size);
		bool createImgae(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags flags, uint32_t mipLevels);

		bool createTexture2DImageFromData(
			VkImage& Texture2DImage, VkDeviceMemory& Texture2DImageMemory,
			void* imageData, uint32_t texWidth, uint32_t texHeight,
			VkFormat texFormat,
			uint32_t layerCount = 1, VkImageCreateFlags flags = 0);

		bool updateTexture2DImage(VkImage& Texture2DImage, VkDeviceMemory& Texture2DImageMemory, 
			uint32_t texWidth, uint32_t texHeight, 
			VkFormat texFormat,
			uint32_t layerCount,
			const void* imageData,
			VkImageLayout sourceImageLayout = VK_IMAGE_LAYOUT_UNDEFINED);

		uint32_t bytesPerTexFormat(VkFormat fmt);

		std::shared_ptr<LxhBuffer> m_imageBuffer;
		VkImage m_image = VK_NULL_HANDLE;
		VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;
		VkSampler m_sampler = VK_NULL_HANDLE;


		VkDescriptorImageInfo m_Descriptor{};


		VkFormat m_format;
		VkExtent3D m_extent;
		uint32_t m_mipLevels;
	};

}
