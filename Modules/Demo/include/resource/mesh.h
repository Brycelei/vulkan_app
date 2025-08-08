#include "lxh_buffer.h"
#include "lxh_device.h"
#include "lxh_texture.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include<memory>

namespace lxh
{
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

	struct Texture
	{
		Texture2D texture;
		std::string type;
		std::string path;
	};

	class Mesh
	{
	public:
		Mesh();
		~Mesh();

		Mesh(const Mesh&);
		Mesh(const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices,
		std::string name, std::vector<Texture> &textures);
		Mesh(std::vector<uint32_t>& indices, std::vector<Vertex>& vertices, 
		std::string name, std::vector<Texture>& textures);

		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&& other) noexcept;
		const std::shared_ptr<LxhBuffer> getVertexBuffer() { return vertexBuffer; }
		const std::shared_ptr<LxhBuffer> getIndexBuffer() { return indexBuffer; }
		void setName(const std::string& name) { m_name = name; }
		const std::string& getName() const { return m_name; }

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

		void createVertexBuffers(LxhDevice& device, const std::vector<Vertex>& vertices);

		void createIndexBuffers(LxhDevice& device, const std::vector<uint32_t>& indices);

		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		
		std::vector<Texture> m_textures;
	private:
	
		std::shared_ptr<LxhBuffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		std::shared_ptr<LxhBuffer> indexBuffer;
		uint32_t indexCount;

		std::string m_name;
	};
}