// LxhMaterial.h
#pragma once

#include "lxh_device.h"
#include "lxh_texture.h"
#include "lxh_descriptors.h"
#include "lxh_model.h"
#include "lxh_buffer.h"
namespace lxh {

	struct MaterialUbo {
		glm::vec4 baseColorFactor; // 基础颜色
		float metallicFactor;      // 金属度
		float roughnessFactor;     // 粗糙度
		// ... 其他材质参数
	};

	class LxhMaterial {
	public:
		LxhMaterial(LxhDescriptorPool& pool, VkDescriptorSetLayout materialSetLayout);
		~LxhMaterial();

		// 禁止拷贝构造和赋值
		LxhMaterial(const LxhMaterial&) = delete;
		LxhMaterial& operator=(const LxhMaterial&) = delete;

		void updateMaterialUbo(MaterialUbo ubo);
		void bindTexture(std::shared_ptr<Texture2D> texture, uint32_t binding = 0);

		VkDescriptorSet getDescriptorSet() const { return materialDescriptorSet; }

	private:
		LxhDevice& lxhDevice; // 单例获取设备实例
		std::unique_ptr<LxhBuffer> materialUboBuffer;
		VkDescriptorSet materialDescriptorSet;
		std::shared_ptr<Texture2D> baseColorTexture;

		lxh::LxhDescriptorPool& descriptorPool; // 描述符池引用
	};
}
