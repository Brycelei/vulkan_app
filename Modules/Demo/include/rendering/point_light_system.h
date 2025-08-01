#pragma once

#include "lxh_camera.h"
#include "lxh_device.h"
#include "lxh_frame_info.h"
#include "lxh_game_object.h"
#include "lxh_pipeline.h"

// std
#include <memory>
#include <vector>

namespace lxh {
	class PointLightSystem {
	public:
		PointLightSystem(
			LxhDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		LxhDevice& lxhDevice;

		std::unique_ptr<LxhPipeline> lxhPipeline;
		VkPipelineLayout pipelineLayout;
	};
}  // namespace lxh
