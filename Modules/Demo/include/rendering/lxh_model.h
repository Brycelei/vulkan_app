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
#include <string>
#include <vector>

namespace lxh {
	class LxhModel {
	public:
		struct Builder {

			void loadModel(const std::string& filepath, LxhDevice& device);

			AssimpParser parser;
			std::vector<std::shared_ptr<Mesh>> m_meshs;

			std::string m_directory;
			std::string m_name;
		};

		LxhModel(LxhDevice& device, const LxhModel::Builder& builder);
		~LxhModel() = default;

		LxhModel(const LxhModel&) = delete;
		LxhModel& operator=(const LxhModel&) = delete;

		static std::unique_ptr<LxhModel> createModelFromFile(
			LxhDevice& device, const std::string& filepath);

		const std::vector<std::shared_ptr<Mesh>>& getMeshes() const { return m_meshs; }

	private:
		LxhDevice& lxhDevice;

		std::vector<std::shared_ptr<Mesh>> m_meshs;
	};
}  // namespace lxh
