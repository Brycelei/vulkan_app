#include <model_loader.h>
#include <lxh_texture.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace lxh
{
	bool AssimpParser::LoadModel(const std::string& p_fileName, LxhDevice& device, std::vector<std::shared_ptr<Mesh>>& p_meshes)
	{
		// The cache only deduplicates textures within a single model load; clearing
		// it up front makes sure the parser never owns GPU resources of a previous load.
		m_cachedTextures.clear();

		Assimp::Importer import;

		/*
		* when use vulkan , must not flip Y
		*/
		const aiScene* scene = import.ReadFile(p_fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			return false;

		m_directory = p_fileName.substr(0, p_fileName.find_last_of("/\\"));
		aiMatrix4x4 identity;

		ProcessNode(identity, scene->mRootNode, scene, device, p_meshes);

		return true;
	}

	void AssimpParser::ProcessNode(const aiMatrix4x4& p_transform, aiNode* p_node, const aiScene* p_scene,
		LxhDevice& device, std::vector<std::shared_ptr<Mesh>>& p_meshes)
	{
		aiMatrix4x4 nodeTransformation = p_transform * p_node->mTransformation;
		// Process all the node's meshes (if any)
		for (uint32_t i = 0; i < p_node->mNumMeshes; ++i)
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::vector<Texture> textures;
			aiMesh* mesh = p_scene->mMeshes[p_node->mMeshes[i]];
			std::string meshName = mesh->mName.C_Str();
			ProcessMesh(nodeTransformation, mesh, p_scene, device, vertices, indices, textures);
			p_meshes.push_back(std::make_shared<Mesh>(std::move(indices), std::move(vertices),
				std::move(meshName), std::move(textures)));
		}

		// Then do the same for each of its children
		for (uint32_t i = 0; i < p_node->mNumChildren; ++i)
		{
			ProcessNode(nodeTransformation, p_node->mChildren[i], p_scene, device, p_meshes);
		}
	}

	void AssimpParser::ProcessMesh(const aiMatrix4x4& p_transform, aiMesh* p_mesh, const aiScene* p_scene,
		LxhDevice& device, std::vector<Vertex>& p_outVertices, std::vector<uint32_t>& p_outIndices,
		std::vector<Texture>& p_textures)
	{
		for (uint32_t i = 0; i < p_mesh->mNumVertices; ++i)
		{
			const aiVector3D position = p_transform * p_mesh->mVertices[i];
			const aiVector3D texCoords = p_mesh->mTextureCoords[0] ? p_mesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);
			const aiVector3D normal = p_transform * (p_mesh->mNormals ? p_mesh->mNormals[i] : aiVector3D(0.0f, 0.0f, 0.0f));
			const aiVector3D tangent = p_transform * (p_mesh->mTangents ? p_mesh->mTangents[i] : aiVector3D(0.0f, 0.0f, 0.0f));
			const aiVector3D bitangent = p_transform * (p_mesh->mBitangents ? p_mesh->mBitangents[i] : aiVector3D(0.0f, 0.0f, 0.0f));

			// Vulkan's Y axis points down compared to the OBJ convention: flip Y for
			// positions AND their basis vectors so normals stay on the outward side.
			p_outVertices.push_back({
				{position.x, -position.y, position.z},
				{texCoords.x, texCoords.y},
				{normal.x, -normal.y, normal.z},
				{tangent.x, -tangent.y, tangent.z},
				{bitangent.x, -bitangent.y, bitangent.z}
				});
		}

		for (uint32_t faceID = 0; faceID < p_mesh->mNumFaces; ++faceID)
		{
			auto& face = p_mesh->mFaces[faceID];

			for (size_t indexID = 0; indexID < 3; ++indexID)
				p_outIndices.push_back(face.mIndices[indexID]);
		}

		// Extract PBR texture slots. Specular maps are intentionally dropped:
		// the Cook-Torrance BRDF derives specular from metallic/roughness instead.
		if (p_mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = p_scene->mMaterials[p_mesh->mMaterialIndex];

			auto albedoTextures = ExtractTextures(material, device, aiTextureType_DIFFUSE, TextureType::Albedo, VK_FORMAT_R8G8B8A8_SRGB);
			p_textures.insert(p_textures.end(), albedoTextures.begin(), albedoTextures.end());
			auto normalTextures = ExtractTextures(material, device, aiTextureType_HEIGHT, TextureType::Normal, VK_FORMAT_R8G8B8A8_UNORM);
			p_textures.insert(p_textures.end(), normalTextures.begin(), normalTextures.end());
			auto normalTextures2 = ExtractTextures(material, device, aiTextureType_NORMALS, TextureType::Normal, VK_FORMAT_R8G8B8A8_UNORM);
			p_textures.insert(p_textures.end(), normalTextures2.begin(), normalTextures2.end());
			auto roughnessTextures = ExtractTextures(material, device, aiTextureType_SHININESS, TextureType::Roughness, VK_FORMAT_R8G8B8A8_UNORM);
			p_textures.insert(p_textures.end(), roughnessTextures.begin(), roughnessTextures.end());
			auto aoTextures = ExtractTextures(material, device, aiTextureType_AMBIENT, TextureType::AO, VK_FORMAT_R8G8B8A8_UNORM);
			p_textures.insert(p_textures.end(), aoTextures.begin(), aoTextures.end());
		}
	}

	std::vector<Texture> AssimpParser::ExtractTextures(aiMaterial* material, LxhDevice& device,
		aiTextureType textureType, const std::string& typeName, VkFormat format)
	{
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < material->GetTextureCount(textureType); ++i)
		{
			aiString textureFilename;
			material->GetTexture(textureType, i, &textureFilename);

			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip = false;
			for (const auto& cached : m_cachedTextures)
			{
				if (cached.path == textureFilename.C_Str())
				{
					textures.push_back(cached);
					skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}

			if (!skip) // if texture hasn't been loaded already, load it
			{
				auto texturePtr = std::make_shared<Texture2D>(device, m_directory + "/" + textureFilename.C_Str(), format);

				Texture tex = { std::move(texturePtr), typeName, std::string(textureFilename.C_Str()) };
				textures.push_back(tex);
				m_cachedTextures.push_back(tex);
			}
		}
		return textures;
	}
}
