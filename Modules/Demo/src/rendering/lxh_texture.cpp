#include <lxh_Texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"
#include <stdexcept>
namespace lxh {

	/*
	* 
	*/
	Texture2D::Texture2D(const std::string& filepath, VkFormat format /*= VK_FORMAT_R8G8B8A8_SRGB*/, uint32_t mipLevels /*= 1*/)
		:m_format(format), m_mipLevels(mipLevels)
	{
		
		createTexture2DImage(filepath.c_str());
		createImageView(m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &m_imageView);
		createTexture2DSampler(&m_sampler);
		UpdateDescriptor();
	}

	Texture2D::Texture2D(VkFormat format, VkExtent3D extent, uint32_t mipLevels)
		: m_format(format), m_extent(extent), m_mipLevels(mipLevels)
	{
		
	}

	//Texture2D::Texture2D(Texture2D&& other) noexcept
	//	:m_image(other.m_image), m_imageMemory(other.m_imageMemory),
	//	m_imageView(other.m_imageView), m_sampler(other.m_sampler), m_format(other.m_format),
	//	m_extent(other.m_extent), m_mipLevels(other.m_mipLevels)
	//{
	//	other.m_image = VK_NULL_HANDLE;
	//	other.m_imageMemory = VK_NULL_HANDLE;
	//	other.m_imageView = VK_NULL_HANDLE;
	//	other.m_sampler = VK_NULL_HANDLE;
	//}

	void Texture2D::createTexture2DImage(const char* filename)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels) {
			throw std::runtime_error("Failed to load Texture2D image: " + std::string(filename));
		}
		m_extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };

		VkDeviceSize imageSize = m_extent.width * m_extent.height * 4;
		bool result = createTexture2DImageFromData(m_image, m_imageMemory,
			pixels, texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM);

		stbi_image_free(pixels);
	}

	

	bool Texture2D::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView, VkImageViewType viewType /*= VK_IMAGE_VIEW_TYPE_2D*/, uint32_t layerCount /*= 1*/, uint32_t mipLevels /*= 1*/)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(LxhDevice::getInstance().getDevice(), &viewInfo, nullptr, &m_imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image view!");
		}
		return true;
	}

	bool Texture2D::createTexture2DSampler(VkSampler* sampler, VkFilter minFilter /*= VK_FILTER_LINEAR*/, VkFilter maxFilter /*= VK_FILTER_LINEAR*/, VkSamplerAddressMode addressMode /*= VK_SAMPLER_ADDRESS_MODE_REPEAT*/)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(LxhDevice::getInstance().getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		if (vkCreateSampler(LxhDevice::getInstance().getDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create Texture2D sampler!");
		}
		return true;
	}


	void Texture2D::createImageBuffer(VkDeviceSize  size)
	{

	}

	bool Texture2D::createImgae(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags flags, uint32_t mipLevels)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = (uint32_t)((flags == VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ? 6 : 1);
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = flags;
		VkResult result = vkCreateImage(LxhDevice::getInstance().getDevice(), &imageInfo, nullptr, &image);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image: " + std::to_string(result));
		}
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(LxhDevice::getInstance().getDevice(), image, &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = LxhDevice::getInstance().findMemoryType(memRequirements.memoryTypeBits, properties);
		result = vkAllocateMemory(LxhDevice::getInstance().getDevice(), &allocInfo, nullptr, &imageMemory);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory: " + std::to_string(result));
		}
		vkBindImageMemory(LxhDevice::getInstance().getDevice(), image, imageMemory, 0);
		return true;
	}

	bool Texture2D::createTexture2DImageFromData(VkImage& Texture2DImage, VkDeviceMemory& Texture2DImageMemory, void* imageData, uint32_t texWidth, uint32_t texHeight, VkFormat texFormat, uint32_t layerCount /*= 1*/, VkImageCreateFlags flags /*= 0*/)
	{
		createImgae(texWidth, texHeight, texFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Texture2DImage, Texture2DImageMemory, flags, m_mipLevels);
		return updateTexture2DImage(Texture2DImage, Texture2DImageMemory, texWidth, texHeight, texFormat, layerCount, imageData, VK_IMAGE_LAYOUT_UNDEFINED);
	}

	bool Texture2D::updateTexture2DImage(VkImage& Texture2DImage, VkDeviceMemory& Texture2DImageMemory, uint32_t texWidth, uint32_t texHeight, VkFormat texFormat, uint32_t layerCount, const void* imageData, VkImageLayout sourceImageLayout /*= VK_IMAGE_LAYOUT_UNDEFINED*/)
	{
		uint32_t bytesPerPixel = bytesPerTexFormat(texFormat);

		VkDeviceSize layerSize = texWidth * texHeight * bytesPerPixel;
		VkDeviceSize imageSize = layerSize * layerCount;
		
		LxhBuffer stagingBuffer{
			LxhDevice::getInstance(),
			imageSize,
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer(imageData);
		m_imageBuffer = std::make_unique<LxhBuffer>(
			LxhDevice::getInstance(),
			imageSize,
			1,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		

		LxhDevice::getInstance().transitionImageLayout(Texture2DImage, texFormat, sourceImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layerCount);
		LxhDevice::getInstance().copyBufferToImage(stagingBuffer.getBuffer(), Texture2DImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), layerCount);
		LxhDevice::getInstance().transitionImageLayout(Texture2DImage, texFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, layerCount);
		
		return true;
	}

	uint32_t Texture2D::bytesPerTexFormat(VkFormat fmt)
	{
		switch (fmt)
		{
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_UNORM:
			return 1;
		case VK_FORMAT_R16_SFLOAT:
			return 2;
		case VK_FORMAT_R16G16_SFLOAT:
			return 4;
		case VK_FORMAT_R16G16_SNORM:
			return 4;
		case VK_FORMAT_B8G8R8A8_UNORM:
			return 4;
		case VK_FORMAT_R8G8B8A8_UNORM:
			return 4;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return 4 * sizeof(uint16_t);
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return 4 * sizeof(float);
		default:
			break;
		}
		return 0;
	}

	Texture2D::~Texture2D()
	{
		/*vkDestroySampler(LxhDevice::getInstance().getDevice(), m_sampler, nullptr);
		vkDestroyImageView(LxhDevice::getInstance().getDevice(), m_imageView, nullptr);
		vkDestroyImage(LxhDevice::getInstance().getDevice(), m_image, nullptr);*/
	}

	void Texture2D::UpdateDescriptor()
	{
		m_Descriptor.sampler = m_sampler;
		m_Descriptor.imageView = m_imageView;
		m_Descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

}