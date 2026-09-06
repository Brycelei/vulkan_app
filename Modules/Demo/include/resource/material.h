#pragma once
#include "lxh_device.h"
#include "lxh_descriptors.h"
#include "lxh_texture.h"

#include <memory>

namespace lxh
{
	// Scalar material factors, pushed per mesh through push constants
	// (packed into a single vec4 in the shader: x=metallic, y=roughness, z=ao).
	struct MaterialParams {
		float metallic = 1.0f;
		float roughness = 1.0f;
		float ao = 1.0f;
		float pad = 0.0f;
	};

	// PBR material for the metallic-roughness workflow. Owns the four texture
	// slots and one descriptor set (set 1) exposing them as combined image
	// samplers: binding 0 = albedo, 1 = normal, 2 = roughness, 3 = AO.
	class LxhMaterial
	{
	public:
		LxhMaterial(
			LxhDescriptorPool& pool,
			LxhDescriptorSetLayout& setLayout,
			std::shared_ptr<Texture2D> albedo,
			std::shared_ptr<Texture2D> normal,
			std::shared_ptr<Texture2D> roughness,
			std::shared_ptr<Texture2D> ao);

		LxhMaterial(const LxhMaterial&) = delete;
		LxhMaterial& operator=(const LxhMaterial&) = delete;
		~LxhMaterial() = default;

		VkDescriptorSet getDescriptorSet() const { return m_descriptorSet; }

		const MaterialParams& getParams() const { return m_params; }
		void setParams(const MaterialParams& params) { m_params = params; }

		const std::shared_ptr<Texture2D>& getAlbedo() const { return m_albedo; }
		const std::shared_ptr<Texture2D>& getNormal() const { return m_normal; }
		const std::shared_ptr<Texture2D>& getRoughness() const { return m_roughness; }
		const std::shared_ptr<Texture2D>& getAo() const { return m_ao; }

	private:
		std::shared_ptr<Texture2D> m_albedo;
		std::shared_ptr<Texture2D> m_normal;
		std::shared_ptr<Texture2D> m_roughness;
		std::shared_ptr<Texture2D> m_ao;

		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
		MaterialParams m_params{};
	};
}
