#include "render_system.h"
#include "material.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>


namespace lxh
{
	// Must match the 128-byte push constant block in pbr_shader.vert/.frag:
	// mat4 model + 3 vec4 normal-matrix columns + vec4 material params.
	struct PbrPushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::vec4 nrmCol0{};
		glm::vec4 nrmCol1{};
		glm::vec4 nrmCol2{};
		glm::vec4 materialParams{};  // x = metallic, y = roughness, z = ao
	};

	RenderSystem::RenderSystem(LxhDevice& device, VkRenderPass renderPass,
		VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout materialSetLayout)
		: lxhDevice{ device }
	{
		createPipelineLayout(globalSetLayout, materialSetLayout);
		createPipeline(renderPass);
	}

	void RenderSystem::renderGameObjects(FrameInfo& frameInfo)
	{
		lxhPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr
		);

		for (auto& kv : frameInfo.gameObjects)
		{
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;

			PbrPushConstantData push{};
			push.modelMatrix = obj.transform.mat4();

			const glm::mat3 normalMatrix = obj.transform.normalMatrix();
			push.nrmCol0 = glm::vec4(normalMatrix[0], 0.f);
			push.nrmCol1 = glm::vec4(normalMatrix[1], 0.f);
			push.nrmCol2 = glm::vec4(normalMatrix[2], 0.f);

			for (const auto& mesh : obj.model->getMeshes())
			{
				const LxhMaterial* material = mesh->material.get();
				if (material == nullptr) continue;  // mesh has no material assigned yet

				const MaterialParams& params = material->getParams();
				push.materialParams = glm::vec4(params.metallic, params.roughness, params.ao, 0.f);

				vkCmdPushConstants(
					frameInfo.commandBuffer,
					pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(PbrPushConstantData),
					&push);

				VkDescriptorSet materialSet = material->getDescriptorSet();
				vkCmdBindDescriptorSets(
					frameInfo.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineLayout,
					1,
					1,
					&materialSet,
					0,
					nullptr);

				mesh->bind(frameInfo.commandBuffer);
				mesh->draw(frameInfo.commandBuffer);
			}
		}
	}

	void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout materialSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PbrPushConstantData);

		// set 0: global UBO, set 1: PBR material samplers
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout, materialSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(lxhDevice.getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void RenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		LxhPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		lxhPipeline = std::make_unique<LxhPipeline>(
			lxhDevice,
			"./shaders/pbr_shader.vert.spv",
			"./shaders/pbr_shader.frag.spv",
			pipelineConfig);
	}

	RenderSystem::~RenderSystem()
	{
		vkDestroyPipelineLayout(lxhDevice.getDevice(), pipelineLayout, nullptr);

	}

}
