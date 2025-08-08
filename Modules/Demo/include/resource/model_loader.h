#pragma once
#include<mesh.h>
#include<core.h>

struct aiMesh;
struct aiScene;
struct aiNode;
struct aiMaterial;
enum  aiTextureType;

namespace lxh
{
	class AssimpParser
	{
	public:
		/**
		* Simply load meshes from a file using assimp
		* Return true on success
		* @param p_filename
		* @param p_meshes
		*/
		bool LoadModel
		(
			const std::string& p_fileName,
			std::vector<std::shared_ptr<Mesh>>& p_meshes,
			std::vector<std::string>& p_materials
		);

	private:
		void ProcessMaterials(const struct aiScene* p_scene, std::vector<std::string>& p_materials);;
		void ProcessNode(void* p_transform, struct aiNode* p_node, const struct aiScene* p_scene, std::vector<std::shared_ptr<Mesh>>& p_meshes);
		void ProcessMesh(void* p_transform, struct aiMesh* p_mesh, const struct aiScene* p_scene, std::vector<Vertex>& p_outVertices, std::vector<uint32_t>& p_outIndices, std::vector<Texture>& p_textures);
		std::vector<Texture> ExtractTextures(const aiScene* scene, aiMaterial* material, aiTextureType textureType, const std::string& typeName);

		std::vector<Texture> m_cachedTextures;
		std::string m_directory;
	};
}