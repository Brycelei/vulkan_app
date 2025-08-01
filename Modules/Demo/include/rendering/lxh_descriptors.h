#pragma once

#include "lxh_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace lxh {

	class LxhDescriptorSetLayout {
	public:
		class Builder {
		public:
			Builder(LxhDevice& lxhDevice) : lxhDevice{ lxhDevice } {}

			Builder& addBinding(
				uint32_t binding,
				VkDescriptorType descriptorType,
				VkShaderStageFlags stageFlags,
				uint32_t count = 1);
			std::unique_ptr<LxhDescriptorSetLayout> build() const;

		private:
			LxhDevice& lxhDevice;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
		};

		LxhDescriptorSetLayout(
			LxhDevice& lxhDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
		~LxhDescriptorSetLayout();
		LxhDescriptorSetLayout(const LxhDescriptorSetLayout&) = delete;
		LxhDescriptorSetLayout& operator=(const LxhDescriptorSetLayout&) = delete;

		VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

	private:
		LxhDevice& lxhDevice;
		VkDescriptorSetLayout descriptorSetLayout;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

		friend class LxhDescriptorWriter;
	};

	class LxhDescriptorPool {
	public:
		class Builder {
		public:
			Builder(LxhDevice& lxhDevice) : lxhDevice{ lxhDevice } {}

			Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
			Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder& setMaxSets(uint32_t count);
			std::unique_ptr<LxhDescriptorPool> build() const;

		private:
			LxhDevice& lxhDevice;
			std::vector<VkDescriptorPoolSize> poolSizes{};
			uint32_t maxSets = 1000;
			VkDescriptorPoolCreateFlags poolFlags = 0;
		};

		LxhDescriptorPool(
			LxhDevice& lveDevice,
			uint32_t maxSets,
			VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize>& poolSizes);
		~LxhDescriptorPool();
		LxhDescriptorPool(const LxhDescriptorPool&) = delete;
		LxhDescriptorPool& operator=(const LxhDescriptorPool&) = delete;

		bool allocateDescriptor(
			const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

		void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

		void resetPool();

	private:
		LxhDevice& lxhDevice;
		VkDescriptorPool descriptorPool;

		friend class LxhDescriptorWriter;
	};

	class LxhDescriptorWriter {
	public:
		LxhDescriptorWriter(LxhDescriptorSetLayout& setLayout, LxhDescriptorPool& pool);

		LxhDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		LxhDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		bool build(VkDescriptorSet& set);
		void overwrite(VkDescriptorSet& set);

	private:
		LxhDescriptorSetLayout& setLayout;
		LxhDescriptorPool& pool;
		std::vector<VkWriteDescriptorSet> writes;
	};

}
