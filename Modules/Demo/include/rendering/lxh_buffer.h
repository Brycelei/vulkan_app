#pragma once

#include <lxh_device.h>

namespace lxh
{
	class LxhBuffer
	{
	public:
		LxhBuffer(
			LxhDevice& device,
			VkDeviceSize instanceSize,
			uint32_t intanceCount,
			VkBufferUsageFlags usageFlags,
			VkMemoryPropertyFlags memoryPropertyFlags,
			VkDeviceSize minOffsetAligment = 1
	);
	~LxhBuffer();
	LxhBuffer(const LxhBuffer &) = delete; 

	LxhBuffer& operator=(const LxhBuffer&) = delete;

	VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	void unmap();

	void writeToBuffer(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
	VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	void writeToIndexBuffer(void* data, uint32_t index);
	VkResult flushIndex(uint32_t index);
	VkDescriptorBufferInfo descriptorIndexInfo(uint32_t index) const;
	VkResult invalidateIndex(uint32_t index);

	VkBuffer getBuffer() const { return buffer_; }
	void* getMappedMemory() const { return mapped; }
	uint32_t getInstanceCount() const { return instanceCount; }
	VkDeviceSize getInstanceSize() const { return instanceSize; }
	VkDeviceSize getAlignmentSize() const { return instanceSize; }
	VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
	VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
	VkDeviceSize getBufferSize() const { return bufferSize; }

	private:
		static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);
		LxhDevice& device;
		void* mapped = nullptr;
		VkBuffer buffer_ = VK_NULL_HANDLE;
		VkDeviceMemory bufferMemory_ = VK_NULL_HANDLE;

		VkDeviceSize bufferSize;
		uint32_t instanceCount;
		VkDeviceSize instanceSize;
		VkDeviceSize alignmentSize;
		VkBufferUsageFlags usageFlags;
		VkMemoryPropertyFlags memoryPropertyFlags;
	};

}