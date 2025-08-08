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
		struct Builder {
		
			void loadModel(const std::string& filepath);

			static AssimpParser __ASSIMP;
			
			std::vector<std::shared_ptr<Mesh>> m_meshs;
			std::vector<std::string> m_materialNames;

			std::string m_directory;
			std::string m_name;
		};

		LxhModel(LxhDevice& device, const LxhModel::Builder& builder);
		~LxhModel();

		LxhModel(const LxhModel&) = delete;
		LxhModel& operator=(const LxhModel&) = delete;

		static std::unique_ptr<LxhModel> createModelFromFile(
			LxhDevice& device, const std::string& filepath);

		void draw(VkCommandBuffer commandbuffer);
		std::vector<std::shared_ptr<Mesh>> getMeshes() const { return m_meshs; }
	private:
		void createVertexBuffers(LxhDevice& device, Mesh & mesh);
		void createIndexBuffers(LxhDevice& device, Mesh& mesh);

		LxhDevice& lxhDevice;

		std::vector<std::shared_ptr<Mesh>> m_meshs;
		std::vector<std::string> m_materialNames;
	};
}  // namespace lxh
