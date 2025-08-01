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
	class RenderSystem {
	public:
		RenderSystem(
			LxhDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~RenderSystem();

		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		LxhDevice& lxhDevice;

		std::unique_ptr<LxhPipeline> lxhPipeline;
		VkPipelineLayout pipelineLayout;
	};
}  // namespace lxh
