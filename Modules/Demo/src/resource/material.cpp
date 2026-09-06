#include <material.h>

#include <stdexcept>

namespace lxh
{
	LxhMaterial::LxhMaterial(
		LxhDescriptorPool& pool,
		LxhDescriptorSetLayout& setLayout,
		std::shared_ptr<Texture2D> albedo,
		std::shared_ptr<Texture2D> normal,
		std::shared_ptr<Texture2D> roughness,
		std::shared_ptr<Texture2D> ao)
		: m_albedo(std::move(albedo))
		, m_normal(std::move(normal))
		, m_roughness(std::move(roughness))
		, m_ao(std::move(ao))
	{
		LxhDescriptorWriter writer(setLayout, pool);
		writer.writeImage(0, &m_albedo->getDescriptorInfo());
		writer.writeImage(1, &m_normal->getDescriptorInfo());
		writer.writeImage(2, &m_roughness->getDescriptorInfo());
		writer.writeImage(3, &m_ao->getDescriptorInfo());
		if (!writer.build(m_descriptorSet))
		{
			throw std::runtime_error("Failed to build material descriptor set!");
		}
	}
}
