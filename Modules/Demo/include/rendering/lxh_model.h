#pragma once

#include "lxh_buffer.h"
#include "lxh_device.h"
#include <model_loader.h>
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace lxh {
	class LxhModel {
	public:
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal &&
					uv == other.uv;
			}
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filepath);

			void loadModelAssimp(const std::string& filepath);
			static AssimpParser __ASSIMP;

		};

		LxhModel(LxhDevice& device, const LxhModel::Builder& builder);
		~LxhModel();

		LxhModel(const LxhModel&) = delete;
		LxhModel& operator=(const LxhModel&) = delete;

		static std::unique_ptr<LxhModel> createModelFromFile(
			LxhDevice& device, const std::string& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		LxhDevice& lxhDevice;

		std::unique_ptr<LxhBuffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		std::unique_ptr<LxhBuffer> indexBuffer;
		uint32_t indexCount;
	};
}  // namespace lxh
