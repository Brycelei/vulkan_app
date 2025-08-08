#pragma once

#include<lxh_application.h>
#include<vector>

namespace lxh
{
	extern lxh::LxhWindow* g_window_ptr; // 你需要在工程中定义并初始化它

	struct SwapChainSupportDeatails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		uint32_t graphicsFamily;
		uint32_t presentFamily;
		bool graphicsFamilyHasValue = false;
		bool presentFamilyHasValue = false;
		bool isComplete() const
		{
			return graphicsFamilyHasValue && presentFamilyHasValue;
		}
	};

	class LxhDevice
	{
	public:
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

 // 单例获取方法
    static LxhDevice& getInstance(LxhWindow& window) {
        static LxhDevice instance(window);
        return instance;
    }
	static LxhDevice& getInstance()
	{
		// 注意：此重载要求先用带参数的 getInstance 初始化一次
		return getInstance(*g_window_ptr);
	}
		
		VkCommandPool getCommandPool() const { return commandPool; }
		VkDevice getDevice() const { return device_; }

		VkSurfaceKHR getSurface() const { return surface_; }
		VkQueue getGraphicsQueue() const { return graphicsQueue_; }
		VkQueue getPresentQueue() const { return presentQueue_; }
		VkPhysicalDevice getPhysicalDevice()const { return physicalDevice_;}

		SwapChainSupportDeatails getSwapChainSupportDetails()
		{
			return querySwapChainSupport(physicalDevice_);
		}
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		QueueFamilyIndices findPhysiclQueueFamilies() {
			return findQueueFamilies(physicalDevice_);
		}
		VkFormat findSupportFormat(
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags flag
		);

		//buffer helper functions
		void createBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& bufferMemory
		);
		
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		void copyBuffer(VkBuffer scrBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void copyBufferToImage(
			VkBuffer buffer,
			VkImage image,
			uint32_t width,
			uint32_t height,
			uint32_t layCount
		);

		void CreateImageWithInfo
		(
			const VkImageCreateInfo& imageInfo,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory
		);
		bool hasStencilComponent(VkFormat format);
		void transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, 
		uint32_t layerCount = 1, uint32_t mipLevels = 1);
		void transitionImageLayoutCmd(
			VkCommandBuffer commandBuffer, VkImage &image, 
			VkFormat format, VkImageLayout oldLayout, 
			VkImageLayout newLayout, uint32_t layerCount = 1, 
			uint32_t mipLevels = 1);

		VkPhysicalDeviceProperties properties;

	private:
		LxhDevice(LxhWindow& window);
		~LxhDevice();
		LxhDevice(const LxhDevice&) = delete;
		LxhDevice& operator=(const LxhDevice&) = delete;
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();

		//helper functions
		bool isDeviceSuitable(VkPhysicalDevice device);
		std::vector<const char*> getRequiredExtensions();
		bool checkValidationLayerSupport();
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void hasGLFWRequiredInstanceExtensions();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDeatails querySwapChainSupport(VkPhysicalDevice device);

		VkInstance instance_;
		VkDebugUtilsMessengerEXT debugMessenger_;
		LxhWindow& window_;
		VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
		VkCommandPool commandPool;

		VkDevice device_;
		VkSurfaceKHR surface_;
		VkQueue graphicsQueue_;
		VkQueue presentQueue_;

		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	};
}