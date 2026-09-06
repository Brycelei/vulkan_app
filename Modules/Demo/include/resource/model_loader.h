#pragma once
#include <mesh.h>

struct aiMesh;
struct aiScene;
struct aiNode;
struct aiMaterial;
template <typename TReal> struct aiMatrix4x4t;
typedef aiMatrix4x4t<float> aiMatrix4x4;
enum aiTextureType;

namespace lxh
{
	class AssimpParser
	{
	public:
		/**
		* Load meshes (with PBR texture slots) from a file using assimp.
		* Textures referenced by the model are uploaded to the GPU immediately.
		* Return true on success
		* @param p_fileName  path to the model file
		* @param device      Vulkan device used to create texture resources
		* @param p_meshes    output meshes
		*/
		bool LoadModel
		(
			const std::string& p_fileName,
			LxhDevice& device,
			std::vector<std::shared_ptr<Mesh>>& p_meshes
		);

	private:
		void ProcessNode(const aiMatrix4x4& p_transform, aiNode* p_node, const aiScene* p_scene,
			LxhDevice& device, std::vector<std::shared_ptr<Mesh>>& p_meshes);
		void ProcessMesh(const aiMatrix4x4& p_transform, aiMesh* p_mesh, const aiScene* p_scene,
			LxhDevice& device, std::vector<Vertex>& p_outVertices, std::vector<uint32_t>& p_outIndices,
			std::vector<Texture>& p_textures);
		std::vector<Texture> ExtractTextures(aiMaterial* material, LxhDevice& device,
			aiTextureType textureType, const std::string& typeName, VkFormat format);

		std::vector<Texture> m_cachedTextures;
		std::string m_directory;
	};
}
