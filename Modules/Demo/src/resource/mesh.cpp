#include<mesh.h>


namespace lxh
{ 
std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;

}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(5);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, Position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, TexCoords);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, Normal);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex, Tangent);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(Vertex, Bitangent);

	return attributeDescriptions;
}


	Mesh::Mesh()
		: m_name("default")
		, indexBuffer(nullptr)
		, vertexBuffer(nullptr)
		, vertexCount(0)
		, indexCount(0)
	{

	}
	Mesh::Mesh(const Mesh& mesh)
		: m_name(mesh.m_name)
		, indexBuffer(mesh.indexBuffer)
		, vertexBuffer(mesh.vertexBuffer)
		, vertexCount(mesh.vertexCount)
		, indexCount(mesh.indexCount)
	{
	}


	Mesh::Mesh(const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices, std::string  name, std::vector<Texture> &textures)
		: indexBuffer(nullptr)
		, vertexBuffer(nullptr)
		, vertexCount(static_cast<uint32_t>(vertices.size()))
		, indexCount(static_cast<uint32_t>(indices.size()))
		,vertices(vertices)
		,indices(indices)
		,m_name(name)
		,m_textures(textures)
	{
		if (indices.size() > 0)
		{
			hasIndexBuffer = true;
		}
	}

	Mesh::Mesh(std::vector<uint32_t>& indices, std::vector<Vertex>& vertices, std::string name, std::vector<Texture> &textures)
		: indexBuffer(nullptr)
		, vertexBuffer(nullptr)
		, vertexCount(static_cast<uint32_t>(vertices.size()))
		, indexCount(static_cast<uint32_t>(indices.size()))
		, vertices(vertices)
		, indices(indices)
		, m_name(name)
		,m_textures(textures)
	{
		if (indices.size() > 0)
		{
			hasIndexBuffer = true;
		}
	}
	
	Mesh::Mesh(Mesh&& other)noexcept
	{
		m_name = std::move(other.m_name);
		indexBuffer = std::move(other.indexBuffer);
		vertexBuffer = std::move(other.vertexBuffer);
		vertexCount = other.vertexCount;
		indexCount = other.indexCount;
		hasIndexBuffer = other.hasIndexBuffer;
		vertices = std::move(other.vertices);
		indices = std::move(other.indices);

		other.vertexCount = 0;
		other.indexCount = 0;
		other.hasIndexBuffer = false;
	}

	void Mesh::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		if (hasIndexBuffer) {
			
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
		else {
			vkCmdBindIndexBuffer(commandBuffer, VK_NULL_HANDLE, 0, VK_INDEX_TYPE_NONE_KHR);
		}
	}

	void Mesh::draw(VkCommandBuffer commandBuffer)
	{
		if (hasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
	}

	void Mesh::createVertexBuffers(LxhDevice &device,const std::vector<Vertex>& vertices)
	{
		assert(vertexCount > 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);
		LxhBuffer stagingBuffer{
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)(vertices.data()));

		vertexBuffer = std::make_unique<LxhBuffer>(
			device,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void Mesh::createIndexBuffers(LxhDevice& device, const std::vector<uint32_t>& indices)
	{
		indexCount = static_cast<uint32_t>(indices.size()); // 修复：确保indexCount和indices一致
		hasIndexBuffer = indexCount > 0;
		if (!hasIndexBuffer)
		{
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		LxhBuffer stagingBuffer{
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)(indices.data()));

		indexBuffer = std::make_unique<LxhBuffer>(
			device,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);

	}

	Mesh::~Mesh()
	{

	}

}