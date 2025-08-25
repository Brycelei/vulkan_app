#include "lxh_material.h"
#include "lxh_descriptors.h"

namespace lxh {
	LxhMaterial::LxhMaterial(LxhDescriptorPool& pool, VkDescriptorSetLayout materialSetLayout)
		: lxhDevice(LxhDevice::getInstance()), descriptorPool(pool)
	{
		// 创建材质UBO
		materialUboBuffer = std::make_unique<LxhBuffer>(
			lxhDevice,
			sizeof(MaterialUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		materialUboBuffer->map();

		auto device = LxhDevice::getInstance().getDevice();
		// 为材质创建描述符集
		LxhDescriptorWriter writer(*LxhDescriptorSetLayout::Builder(lxhDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(),
			pool);

		VkDescriptorBufferInfo bufferInfo = materialUboBuffer->descriptorInfo();
		writer.writeBuffer(0, &bufferInfo);

		// 纹理绑定将在 bindTexture 中处理
		writer.build(materialDescriptorSet);
	}

	LxhMaterial::~LxhMaterial()
	{
	}

	void LxhMaterial::updateMaterialUbo(MaterialUbo ubo) {
		materialUboBuffer->writeToBuffer(&ubo);
	}

	void LxhMaterial::bindTexture(std::shared_ptr<Texture2D> texture, uint32_t binding) {
		baseColorTexture = texture;
		VkDescriptorImageInfo imageInfo = texture->GetDescriptorRef();

		// 更新描述符集中的纹理绑定
		LxhDescriptorWriter writer(*LxhDescriptorSetLayout::Builder(lxhDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(), // 临时构建一个布局，实际应该传入已有的布局
			descriptorPool);

		writer.writeImage(binding, &imageInfo);
		writer.overwrite(materialDescriptorSet);
	}
}