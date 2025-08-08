
#include <lxh_model.h>
#include "lxh_utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include<tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <cstring>
#include <unordered_map>

namespace std {
	template <>
	struct hash<lxh::Vertex> {
		size_t operator()(lxh::Vertex const& vertex) const {
			size_t seed = 0;
			lxh::hashCombine(seed, vertex.Position, vertex.TexCoords, vertex.Normal, vertex.Tangent, vertex.Bitangent);
			return seed;
		}
	};
}  // namespace std


#ifndef ENGINE_DIR
#define ENGINE_DIR ""
#endif

namespace lxh
{
	AssimpParser LxhModel::Builder::__ASSIMP;
	
	void LxhModel::Builder::loadModel(const std::string& filepath)
	{
		if (!__ASSIMP.LoadModel(filepath, m_meshs, m_materialNames))
		{
			return;
		}

		m_directory = filepath.substr(0, filepath.find_last_of('/'));
		m_name = filepath.substr(filepath.find_last_of("/\\") + 1);

		return;
	}
	LxhModel::LxhModel(LxhDevice& device, const LxhModel::Builder& builder):
		lxhDevice(device),
		m_meshs(builder.m_meshs),m_materialNames(builder.m_materialNames)
	{
		for (auto& mesh : m_meshs)
		{
			createVertexBuffers(device, *mesh);
			createIndexBuffers(device, *mesh);
		}
		
	}

	LxhModel::~LxhModel()
	{

	}

	std::unique_ptr<LxhModel> LxhModel::createModelFromFile(LxhDevice& device, const std::string& filepath)
	{
		Builder builder{};
		builder.loadModel(ENGINE_DIR + filepath);
		return std::make_unique<LxhModel>(device, builder);
	}

	void LxhModel::draw(VkCommandBuffer commandbuffer)
	{
		for (auto& mesh : m_meshs)
		{
			mesh->bind(commandbuffer);
			mesh->draw(commandbuffer);
		}
	}

	void LxhModel::createVertexBuffers(LxhDevice& device, Mesh& mesh)
	{
		mesh.createVertexBuffers(device, mesh.vertices);
	}

	void LxhModel::createIndexBuffers(LxhDevice& device, Mesh& mesh)
	{
		mesh.createIndexBuffers(device, mesh.indices);
	}

}
