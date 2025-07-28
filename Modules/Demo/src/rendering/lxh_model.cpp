
#include <lxh_model.h>

namespace lxh
{


	std::vector<VkVertexInputBindingDescription> LxhModel::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;

	}

	std::vector<VkVertexInputAttributeDescription> LxhModel::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, normal);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, uv);

		return attributeDescriptions;
	}

	void LxhModel::Builder::loadModel(const std::string& filepath)
	{

	}

	LxhModel::LxhModel(LxhDevice& device, const LxhModel::Builder& builder):
		lxhDevice(device), vertexCount(static_cast<uint32_t>(builder.vertices.size())),
		indexCount(static_cast<uint32_t>(builder.indices.size()))
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	LxhModel::~LxhModel()
	{

	}

	std::unique_ptr<LxhModel> LxhModel::createModelFromFile(LxhDevice& device, const std::string& filepath)
	{
		Builder builder{};
		builder.loadModel(filepath);
		return std::make_unique<LxhModel>(device, builder);
	}

	void LxhModel::bind(VkCommandBuffer commandBuffer)
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

	void LxhModel::draw(VkCommandBuffer commandBuffer)
	{
		if (hasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
	}

	void LxhModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		assert(vertexCount > 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize  = sizeof(vertices[0])* vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		LxhBuffer stagingBuffer{
			lxhDevice,
			bufferSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)(vertices.data()));

		vertexBuffer = std::make_unique<LxhBuffer>(
			lxhDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		lxhDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	void LxhModel::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		hasIndexBuffer = indexCount > 0;
		if (!hasIndexBuffer)
		{
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0])* indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		LxhBuffer stagingBuffer{
			lxhDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)(indices.data()));

		indexBuffer = std::make_unique<LxhBuffer>(
			lxhDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		lxhDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);

	}

}
