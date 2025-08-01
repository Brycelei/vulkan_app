#pragma once

#include "lxh_device.h"
#include "lxh_swap_chain.h"
#include "lxh_application.h"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace lxh {
	class LxhRenderer {
	public:
		LxhRenderer(LxhWindow& window, LxhDevice& device);
		~LxhRenderer();

		LxhRenderer(const LxhRenderer&) = delete;
		LxhRenderer& operator=(const LxhRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
		float getAspectRatio() const { return swapChain->getExtentAspentRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderpass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderpass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		LxhWindow& window;
		LxhDevice& device;
		std::unique_ptr<LxhSwapChain> swapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{ false };
	};
}
