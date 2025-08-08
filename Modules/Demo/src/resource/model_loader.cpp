

#include<model_loader.h>
#include <lxh_texture.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include<iostream>

namespace lxh
{
	

	bool AssimpParser::LoadModel(const std::string& p_fileName, std::vector<std::shared_ptr<Mesh>>& p_meshes, std::vector<std::string>& p_materials)
	{
		Assimp::Importer import;


		const aiScene * scene = import.ReadFile(p_fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			return false;

		//ProcessMaterials(scene, p_materials);
		m_directory = p_fileName.substr(0, p_fileName.find_last_of("/"));
		aiMatrix4x4 identity;

		ProcessNode(&identity, scene->mRootNode, scene, p_meshes);

		return true;

	}

	void AssimpParser::ProcessMaterials(const aiScene* p_scene, std::vector<std::string>& p_materials)
	{
		for (uint32_t i = 0; i < p_scene->mNumMaterials; ++i)
		{
			aiMaterial* material = p_scene->mMaterials[i];
			if (material)
			{
				aiString name;
				aiGetMaterialString(material, AI_MATKEY_NAME, &name);
				p_materials.push_back(name.C_Str());
			}
		}
	}
	void AssimpParser::ProcessNode(void* p_transform, aiNode* p_node, const aiScene* p_scene, std::vector<std::shared_ptr<Mesh>>& p_meshes)
	{
		aiMatrix4x4 nodeTransformation = *reinterpret_cast<aiMatrix4x4*>(p_transform) * p_node->mTransformation;
		// Process all the node's meshes (if any)
		for (uint32_t i = 0; i < p_node->mNumMeshes; ++i)
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::vector<Texture> textures;
			aiMesh* mesh = p_scene->mMeshes[p_node->mMeshes[i]];
			std::string materialName = mesh->mName.C_Str();
			ProcessMesh(&nodeTransformation, mesh, p_scene, vertices, indices, textures);
			std::shared_ptr<Mesh> meshtemp = std::make_shared<Mesh>(indices, vertices, materialName,textures);
			p_meshes.push_back(meshtemp); // The model will handle mesh destruction
		}

		// Then do the same for each of its children
		for (uint32_t i = 0; i < p_node->mNumChildren; ++i)
		{
			ProcessNode(&nodeTransformation, p_node->mChildren[i], p_scene, p_meshes);
		}
	}
	void AssimpParser::ProcessMesh(void* p_transform, struct aiMesh* p_mesh, 
	const struct aiScene* p_scene, std::vector<Vertex>& p_outVertices, std::vector<uint32_t>& p_outIndices, 
	std::vector<Texture>& p_textures)
	{
		aiMatrix4x4 meshTransformation = *reinterpret_cast<aiMatrix4x4*>(p_transform);

		for (uint32_t i = 0; i < p_mesh->mNumVertices; ++i)
		{
			const aiVector3D position = meshTransformation * p_mesh->mVertices[i];
			const aiVector3D texCoords = p_mesh->mTextureCoords[0] ? p_mesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);
			const aiVector3D normal = meshTransformation * (p_mesh->mNormals ? p_mesh->mNormals[i] : aiVector3D(0.0f, 0.0f, 0.0f));
			const aiVector3D tangent = meshTransformation * (p_mesh->mTangents ? p_mesh->mTangents[i] : aiVector3D(0.0f, 0.0f, 0.0f));
			const aiVector3D bitangent = meshTransformation * (p_mesh->mBitangents ? p_mesh->mBitangents[i] : aiVector3D(0.0f, 0.0f, 0.0f));

			//for vulkan, using left hand coordinate
			glm::vec3 temppositon{ position.x, -position.y, position.z };
			glm::vec2 temptexCoords{ texCoords.x, texCoords.y };
			glm::vec3 tempnormal{ normal.x, normal.y, normal.z };
			glm::vec3 temptangent{ tangent.x, tangent.y, tangent.z };
			glm::vec3 tempbitangent{ bitangent.x, bitangent.y, bitangent.z };

			p_outVertices.push_back({
				temppositon,
				temptexCoords,
				tempnormal,
				temptangent,
				tempbitangent
				});
		}

		for (uint32_t faceID = 0; faceID < p_mesh->mNumFaces; ++faceID)
		{
			auto& face = p_mesh->mFaces[faceID];

			for (size_t indexID = 0; indexID < 3; ++indexID)
				p_outIndices.push_back(face.mIndices[indexID]);
		}

		// Process textures
		if (p_mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = p_scene->mMaterials[p_mesh->mMaterialIndex];

			auto diffuseTextures = ExtractTextures(p_scene, material, aiTextureType_DIFFUSE, "texture_diffuse");
			p_textures.insert(p_textures.end(), diffuseTextures.begin(), diffuseTextures.end());
			auto specularTextures = ExtractTextures(p_scene, material, aiTextureType_SPECULAR, "texture_specular");
			p_textures.insert(p_textures.end(), specularTextures.begin(), specularTextures.end());
			auto normalTextures = ExtractTextures(p_scene, material, aiTextureType_HEIGHT, "texture_normal");
			p_textures.insert(p_textures.end(), normalTextures.begin(), normalTextures.end());
		}
	}

	std::vector<Texture> AssimpParser::ExtractTextures(const aiScene* scene, aiMaterial* material, aiTextureType textureType, const std::string& typeName)
	{
		std::vector<Texture> textures;
		unsigned int j = material->GetTextureCount(textureType);
		for (unsigned int i = 0; i < material->GetTextureCount(textureType); ++i)
		{
			aiString textureFilename;
			material->GetTexture(textureType, i, &textureFilename);

			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip = false;
			for (unsigned int j = 0; j < m_cachedTextures.size(); ++j)
			{
				if (std::strcmp(m_cachedTextures[j].path.data(), textureFilename.C_Str()) == 0)
				{
					textures.emplace_back(m_cachedTextures[j]);
					skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}

			if (!skip) // if texture hasn't been loaded already, load it
			{
		
				Texture2D texture2D = Texture2D(std::string(m_directory + "/" + std::string(textureFilename.C_Str())));

				Texture tex = { texture2D, typeName, std::string(textureFilename.C_Str()) };
				textures.push_back(tex);
				m_cachedTextures.push_back(tex);
			}
		}
		return textures;
	}

}