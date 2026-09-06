#pragma once
#include "lxh_buffer.h"
#include "lxh_device.h"
#include "lxh_texture.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>

namespace lxh
{
	class LxhMaterial;

	struct Vertex {
		glm::vec3 Position{};
		glm::vec2 TexCoords{};
		glm::vec3 Normal{};
		glm::vec3 Tangent{};
		glm::vec3 Bitangent{};

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

		bool operator==(const Vertex& other) const {
			return Position == other.Position && Tangent == other.Tangent && Normal == other.Normal &&
				TexCoords == other.TexCoords && Bitangent == other.Bitangent;
		}
	};

	// Texture slot tags filled in by the model loader; App::createMaterials()
	// turns these into a LxhMaterial (PBR descriptor set) per mesh.
	namespace TextureType
	{
		constexpr const char* Albedo = "texture_albedo";
		constexpr const char* Normal = "texture_normal";
		constexpr const char* Roughness = "texture_roughness";
		constexpr const char* AO = "texture_ao";
	}

	struct Texture
	{
		std::shared_ptr<Texture2D> texture;
		std::string type;
		std::string path;

		Texture(std::shared_ptr<Texture2D> texture2D, std::string type, std::string filename)
			: texture(std::move(texture2D)), type(std::move(type)), path(std::move(filename)) {}
	};

	class Mesh
	{
	public:
		Mesh(std::vector<uint32_t> indices, std::vector<Vertex> vertices,
			std::string name, std::vector<Texture> textures);
		~Mesh() = default;

		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&& other) noexcept;

		const std::shared_ptr<LxhBuffer>& getVertexBuffer() const { return vertexBuffer; }
		const std::shared_ptr<LxhBuffer>& getIndexBuffer() const { return indexBuffer; }
		void setName(const std::string& name) { m_name = name; }
		const std::string& getName() const { return m_name; }

		void bind(VkCommandBuffer commandBuffer) const;
		void draw(VkCommandBuffer commandBuffer) const;

		void createVertexBuffers(LxhDevice& device);
		void createIndexBuffers(LxhDevice& device);

		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		std::vector<Texture> m_textures;
		std::shared_ptr<LxhMaterial> material;  // assigned by App::createMaterials()

	private:
		std::shared_ptr<LxhBuffer> vertexBuffer;
		uint32_t vertexCount = 0;

		bool hasIndexBuffer = false;
		std::shared_ptr<LxhBuffer> indexBuffer;
		uint32_t indexCount = 0;

		std::string m_name;
	};
}
