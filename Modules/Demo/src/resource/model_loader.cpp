

#include<model_loader.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include<iostream>

namespace lxh
{
	
	EModelParserFlags FixFlags(EModelParserFlags p_flags)
	{
		using enum lxh::EModelParserFlags;

		if (static_cast<bool>(p_flags & GEN_NORMALS) && static_cast<bool>(p_flags & GEN_SMOOTH_NORMALS))
		{
			p_flags &= ~GEN_NORMALS;
			std::cout << "AssimpParser: GEN_NORMALS and GEN_SMOOTH_NORMALS are mutually exclusive. GEN_NORMALS will be ignored.";
		}

		if (static_cast<bool>(p_flags & OPTIMIZE_GRAPH) && static_cast<bool>(p_flags & PRE_TRANSFORM_VERTICES))
		{
			p_flags &= ~OPTIMIZE_GRAPH;
			std::cout<<"AssimpParser: OPTIMIZE_GRAPH and PRE_TRANSFORM_VERTICES are mutually exclusive. OPTIMIZE_GRAPH will be ignored.";
		}

		return p_flags;
	}

	bool AssimpParser::LoadModel(const std::string& p_fileName, std::vector<Mesh*>& p_meshes, std::vector<std::string>& p_materials, EModelParserFlags p_parserFlags)
	{
		Assimp::Importer import;

		// Fix the flags to avoid conflicts/invalid scenarios.
		// This is a workaround, ideally the editor UI should not allow this to happen.
		p_parserFlags = FixFlags(p_parserFlags);

		const aiScene* scene = import.ReadFile(p_fileName, static_cast<unsigned int>(p_parserFlags));

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			return false;

		ProcessMaterials(scene, p_materials);

		aiMatrix4x4 identity;

		ProcessNode(&identity, scene->mRootNode, scene, p_meshes);

		return true;

	}
	void AssimpParser::ProcessMaterials(const aiScene* p_scene, std::vector<std::string>& p_materials)
	{
	}
	void AssimpParser::ProcessNode(void* p_transform, aiNode* p_node, const aiScene* p_scene, std::vector<Mesh*>& p_meshes)
	{
		aiMatrix4x4 nodeTransformation = *reinterpret_cast<aiMatrix4x4*>(p_transform) * p_node->mTransformation;
		// Process all the node's meshes (if any)
		for (uint32_t i = 0; i < p_node->mNumMeshes; ++i)
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			aiMesh* mesh = p_scene->mMeshes[p_node->mMeshes[i]];
			ProcessMesh(&nodeTransformation, mesh, p_scene, vertices, indices);
			p_meshes.push_back(new Mesh(indices, vertices)); // The model will handle mesh destruction
		}

		// Then do the same for each of its children
		for (uint32_t i = 0; i < p_node->mNumChildren; ++i)
		{
			ProcessNode(&nodeTransformation, p_node->mChildren[i], p_scene, p_meshes);
		}
	}
	void AssimpParser::ProcessMesh(void* p_transform, aiMesh* p_mesh, const aiScene* p_scene, std::vector<Vertex>& p_outVertices, std::vector<uint32_t>& p_outIndices)
	{
		aiMatrix4x4 meshTransformation = *reinterpret_cast<aiMatrix4x4*>(p_transform);

		for (uint32_t i = 0; i < p_mesh->mNumVertices; ++i)
		{
			const aiVector3D position = meshTransformation * p_mesh->mVertices[i];
			const aiVector3D texCoords = p_mesh->mTextureCoords[0] ? p_mesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);
			const aiVector3D normal = meshTransformation * (p_mesh->mNormals ? p_mesh->mNormals[i] : aiVector3D(0.0f, 0.0f, 0.0f));
			const aiVector3D tangent = meshTransformation * (p_mesh->mTangents ? p_mesh->mTangents[i] : aiVector3D(0.0f, 0.0f, 0.0f));
			const aiVector3D bitangent = meshTransformation * (p_mesh->mBitangents ? p_mesh->mBitangents[i] : aiVector3D(0.0f, 0.0f, 0.0f));


			glm::vec3 temppositon{position.x, position.y, position.z};
			glm::vec2 temptexCoords{ texCoords.x, texCoords.y };
			glm::vec3 tempnormal{normal.x, normal.y, normal.z};
			glm::vec3 temptangent{tangent.x, tangent.y, tangent.z};
			glm::vec3 tempbitangent{ bitangent.x, bitangent.y, bitangent.z};

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
	}
}