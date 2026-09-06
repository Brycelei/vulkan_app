#include <lxh_texture.h>
#include <lxh_buffer.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdexcept>

namespace lxh {

	Texture2D::Texture2D(LxhDevice& device, const std::string& filepath, VkFormat format, uint32_t mipLevels)
		: m_device(device), m_format(format), m_mipLevels(mipLevels)
	{
		int texWidth = 0, texHeight = 0, texChannels = 0;
		stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels)
		{
			throw std::runtime_error("Failed to load Texture2D image: " + filepath);
		}
		m_extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };

		createFromPixels(pixels);
		stbi_image_free(pixels);
	}

	Texture2D::Texture2D(LxhDevice& device, const std::vector<unsigned char>& pixels, VkExtent3D extent, VkFormat format, uint32_t mipLevels)
		: m_device(device), m_format(format), m_extent(extent), m_mipLevels(mipLevels)
	{
		const VkDeviceSize expected = static_cast<VkDeviceSize>(m_extent.width) * m_extent.height * bytesPerFormat(m_format);
		if (pixels.size() < expected)
		{
			throw std::runtime_error("Texture2D pixel buffer is smaller than the given extent!");
		}
		createFromPixels(pixels.data());
	}

	void Texture2D::createFromPixels(const unsigned char* pixels)
	{
		const VkDeviceSize imageSize = static_cast<VkDeviceSize>(m_extent.width) * m_extent.height * bytesPerFormat(m_format);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent = m_extent;
		imageInfo.mipLevels = m_mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = m_format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		m_device.CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);

		// Upload through a host-visible staging buffer
		LxhBuffer stagingBuffer{
			m_device,
			imageSize,
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		stagingBuffer.map();
		stagingBuffer.writeToBuffer(pixels, imageSize);

		m_device.transitionImageLayout(m_image, m_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, m_mipLevels);
		m_device.copyBufferToImage(stagingBuffer.getBuffer(), m_image, m_extent.width, m_extent.height, 1);
		m_device.transitionImageLayout(m_image, m_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, m_mipLevels);

		createImageView();
		createSampler();

		m_descriptor.sampler = m_sampler;
		m_descriptor.imageView = m_imageView;
		m_descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	void Texture2D::createImageView()
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = m_mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_device.getDevice(), &viewInfo, nullptr, &m_imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image view!");
		}
	}

	void Texture2D::createSampler()
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_device.getPhysicalDevice(), &properties);

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
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(m_mipLevels);

		if (vkCreateSampler(m_device.getDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create Texture2D sampler!");
		}
	}

	uint32_t Texture2D::bytesPerFormat(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_SINT:
			return 1;
		case VK_FORMAT_R16_SFLOAT:
			return 2;
		case VK_FORMAT_R16G16_SFLOAT:
		case VK_FORMAT_R16G16_SNORM:
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SRGB:
		case VK_FORMAT_B8G8R8A8_UNORM:
			return 4;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return 8;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return 16;
		default:
			break;
		}
		return 0;
	}

	Texture2D::~Texture2D()
	{
		vkDestroySampler(m_device.getDevice(), m_sampler, nullptr);
		vkDestroyImageView(m_device.getDevice(), m_imageView, nullptr);
		vkDestroyImage(m_device.getDevice(), m_image, nullptr);
		vkFreeMemory(m_device.getDevice(), m_imageMemory, nullptr);
	}

}
