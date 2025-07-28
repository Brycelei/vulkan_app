#include <lxh_buffer.h>

#include <cassert>
#include <cstring>
namespace lxh
{
	/*
		if instanceSize=12, minOffsetAligment = 8;

		12 + 8 - 1 = 19
		~7，  正数的二进制为本身，即0000 0000 0000 0111， 取反为1111 1111 1111 1000
		19的二进制表示为 0000 0000 0001 0011
		 1111 1111 1111 1000
		&0000 0000 0001 0011
	    =0000 0000 0001 0000
		为16
	*/
	VkDeviceSize LxhBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
	{
		if (minOffsetAlignment > 0)
		{
			return (instanceSize + minOffsetAlignment - 1)& ~(minOffsetAlignment - 1);
		}
		return instanceSize;
	}

	LxhBuffer::LxhBuffer(
		LxhDevice& device,
		VkDeviceSize instanceSize,
		uint32_t instanceCount,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkDeviceSize minOffsetAlignment)
		: device(device), instanceCount(instanceCount), instanceSize(instanceSize),
		usageFlags(usageFlags), memoryPropertyFlags(memoryPropertyFlags)
	{
		alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
		bufferSize = alignmentSize * instanceCount;

		device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer_, bufferMemory_);
	}

	VkResult LxhBuffer::map(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/)
	{
		assert(buffer_ && bufferMemory_ && "Called map on buffer before create");
		return vkMapMemory(device.getDevice(), bufferMemory_, offset, size, 0, &mapped);
	}

	void LxhBuffer::unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(device.getDevice(), bufferMemory_);
			mapped = nullptr;
		}
	}

	void LxhBuffer::writeToBuffer(const void* data, VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/)
	{
		assert(mapped && "Cannot copy to unmapped buffer");
		if (size == VK_WHOLE_SIZE)
		{
			memcpy(mapped, data, bufferSize);
		}
		else
		{
			char * memOffset = (char *)mapped;
			memOffset += offset;
			memcpy(mapped, data, size);
		}
	}

	VkResult LxhBuffer::flush(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/)
	{
		VkMappedMemoryRange mappedRange{};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = bufferMemory_;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(device.getDevice(), 1, &mappedRange);
	}

	VkDescriptorBufferInfo LxhBuffer::descriptorInfo(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/) const
	{
		return VkDescriptorBufferInfo(buffer_, offset, size);
	}

	VkResult LxhBuffer::invalidate(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/)
	{
		VkMappedMemoryRange mappedRange{};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = bufferMemory_;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(device.getDevice(), 1, &mappedRange);
	}

	void LxhBuffer::writeToIndexBuffer(void* data, uint32_t index)
	{
		writeToBuffer(data, instanceSize, index * alignmentSize);
	}

	VkResult LxhBuffer::flushIndex(uint32_t index)
	{
		return flush(alignmentSize, index * alignmentSize);
	}

	VkDescriptorBufferInfo LxhBuffer::descriptorIndexInfo(uint32_t index) const
	{
		return descriptorInfo(alignmentSize, index * alignmentSize);	
	}

	VkResult LxhBuffer::invalidateIndex(uint32_t index)
	{
		return invalidate(alignmentSize, index * alignmentSize);
	}

	LxhBuffer::~LxhBuffer()
	{
		unmap();
		vkDestroyBuffer(device.getDevice(), buffer_, nullptr);
		vkFreeMemory(device.getDevice(), bufferMemory_, nullptr);
	}
} // namespace lxh