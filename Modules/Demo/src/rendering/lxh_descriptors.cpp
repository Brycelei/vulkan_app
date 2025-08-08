#include "lxh_descriptors.h"

// std
#include <cassert>
#include <stdexcept>
#include "lxh_swap_chain.h"


namespace lxh
{
	// *************** Descriptor Set Layout Builder *********************

	LxhDescriptorSetLayout::Builder& LxhDescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count /*= 1*/)
	{
		assert(bindings.count(binding) == 0 && "Binding already in use");
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		bindings[binding] = layoutBinding;
		return *this;
	}
	std::unique_ptr<LxhDescriptorSetLayout> LxhDescriptorSetLayout::Builder::build() const 
		{
			return std::make_unique<LxhDescriptorSetLayout>(lxhDevice, bindings);
		}

	LxhDescriptorSetLayout::LxhDescriptorSetLayout(LxhDevice& lxhDevice,
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
			:lxhDevice{ lxhDevice }, bindings{ bindings }
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
			for (auto const kv : bindings) {
				setLayoutBindings.push_back(kv.second);
			}

			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
			descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
			descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

			if (vkCreateDescriptorSetLayout(
				lxhDevice.getDevice(),
				&descriptorSetLayoutInfo,
				nullptr,
				&descriptorSetLayout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}

		}

		LxhDescriptorSetLayout::~LxhDescriptorSetLayout()
		{
			vkDestroyDescriptorSetLayout(lxhDevice.getDevice(), descriptorSetLayout, nullptr);
		}


	LxhDescriptorPool::LxhDescriptorPool(LxhDevice& Device, uint32_t maxSets, 
	VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes)
	:lxhDevice(Device)
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		if (vkCreateDescriptorPool(lxhDevice.getDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	LxhDescriptorPool::~LxhDescriptorPool()
	{
		vkDestroyDescriptorPool(lxhDevice.getDevice(), descriptorPool, nullptr);
	}

	bool LxhDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		
		if (vkAllocateDescriptorSets(lxhDevice.getDevice(), &allocInfo, &descriptor) != VK_SUCCESS)
		{
			return false;
		}
		return true;
	}

	bool LxhDescriptorPool::allocateDescriptors(std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, std::vector<VkDescriptorSet>& descriptors) const
	{
		assert(descriptorSetLayouts.size() == descriptors.size() && "Descriptor set layouts and descriptors size mismatch");

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		allocInfo.pSetLayouts = descriptorSetLayouts.data();

		if (vkAllocateDescriptorSets(lxhDevice.getDevice(), &allocInfo, descriptors.data()) != VK_SUCCESS)
		{
			return false;
		}
		return true;
	}

	void LxhDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
	{
		vkFreeDescriptorSets(lxhDevice.getDevice(), descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
	}

	void LxhDescriptorPool::resetPool()
	{
		vkResetDescriptorPool(lxhDevice.getDevice(), descriptorPool, 0);
	}

	// ***********************descriptor pool builder*********************************
	/*
		this function is used to build a descriptor pool
		the coutn equals swapchain max frame in flight * count
	*/
	LxhDescriptorPool::Builder& LxhDescriptorPool::Builder::addPoolSize(
	VkDescriptorType descriptorType, uint32_t count)
	{
		poolSizes.push_back({ descriptorType, count });
		return *this;
	}
	LxhDescriptorPool::Builder& LxhDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
	{
		poolFlags = flags;
		return *this;
	}

	LxhDescriptorPool::Builder& LxhDescriptorPool::Builder::setMaxSets(uint32_t count)
	{
		maxSets = count;
		return *this;
	}

	std::unique_ptr<LxhDescriptorPool> LxhDescriptorPool::Builder::build() const {
		return std::make_unique<LxhDescriptorPool>(lxhDevice, maxSets, poolFlags, poolSizes);
	}

	// ***********************descriptor writer*********************************

	LxhDescriptorWriter::LxhDescriptorWriter(LxhDescriptorSetLayout& setLayout, LxhDescriptorPool& pool)
		:setLayout{setLayout}, pool{pool}
	{

	}

	LxhDescriptorWriter& LxhDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
	{
		assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
		auto& bindingDescription = setLayout.bindings[binding];
		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = bufferInfo;
		write.descriptorCount = 1;

		writes.push_back(write);
		return *this;
	}

	LxhDescriptorWriter& LxhDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
	{
		assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
		auto &bindingDescription = setLayout.bindings[binding];
		assert(
			bindingDescription.descriptorCount == 1 &&
			"Binding single descriptor info, but binding expects multiple");

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = imageInfo;
		write.descriptorCount = 1;
		
		writes.emplace_back(write);
		return *this;
	}

	bool LxhDescriptorWriter::build(VkDescriptorSet& set)
	{
		bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
		if (!success) {
			return false;
		}
		overwrite(set);
		return true;
	}
	LxhDescriptorWriter& LxhDescriptorWriter::builds(std::vector<VkDescriptorSet>& sets)
	{
		std::vector<VkDescriptorSetLayout> layouts(LxhSwapChain::MAX_FRAME_IN_FLIGHT, setLayout.getDescriptorSetLayout());
		bool success = pool.allocateDescriptors(layouts, sets);
		if (!success) {
			throw std::runtime_error("failed to allocate descriptors!");
		}
		for (auto& set : sets) {
			overwrite(set);
		}
		return *this;
	}

	void LxhDescriptorWriter::overwrite(VkDescriptorSet& set)
	{
		for (auto& write : writes) {
			write.dstSet = set;
		}
		vkUpdateDescriptorSets(pool.lxhDevice.getDevice(), writes.size(), writes.data(), 0, nullptr);
	
	}

}

	
	


