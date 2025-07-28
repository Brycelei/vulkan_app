#pragma once
#include<lxh_device.h>

#include<string>
#include<vector>

namespace lxh
{
	struct PipelineConfigInfo
	{
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		/*VkPipelineShaderStageCreateInfo vertexShaderInfo;
		VkPipelineShaderStageCreateInfo fragShaderInfo;

		VkPipelineVertexInputStateCreateInfo inputInfo;
		VkPipelineInputAssemblyStateCreateInfo assembleInfo;
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineRasterizationStateCreateInfo rasterizer;

		VkPipelineMultisampleStateCreateInfo mutisampleInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

		VkPipelineColorBlendStateCreateInfo blendInfo;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo;*/

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;

	};
	class LxhPipeline {
	public:
		LxhPipeline(
			LxhDevice& device,
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const PipelineConfigInfo& configInfo);
		~LxhPipeline();

		LxhPipeline(const LxhPipeline&) = delete;
		LxhPipeline& operator=(const LxhPipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		static void enableAlphaBlending(PipelineConfigInfo& configInfo);

	private:
		static std::vector<char> readFile(const std::string& filepath);

		void createGraphicPipeline(
			const std::string vertFilepath,
			const std::string fragFilepath,
			const PipelineConfigInfo& configInfo);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		LxhDevice& lxhDevice;
		VkPipeline graphicsPileline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
	};
}