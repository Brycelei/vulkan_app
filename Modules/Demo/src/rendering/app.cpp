#include <app.h>

#include "keyboard_movement_controller.h"
#include "lxh_buffer.h"
#include "lxh_camera.h"
#include "lxh_model.h"
#include "material.h"
#include "mesh.h"
#include "point_light_system.h"
#include "render_system.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

namespace lxh
{
	App::App()
	{
		loadGameObjects();

		// Size the pool for the global UBO sets (one per frame in flight)
		// plus one material set with 4 samplers per mesh.
		uint32_t meshCount = 0;
		for (const auto& kv : gameObjects) {
			if (kv.second.model != nullptr) {
				meshCount += static_cast<uint32_t>(kv.second.model->getMeshes().size());
			}
		}

		globalPool = LxhDescriptorPool::Builder(lxhDevice)
			.setMaxSets(LxhSwapChain::MAX_FRAME_IN_FLIGHT + meshCount)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LxhSwapChain::MAX_FRAME_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 * meshCount)
			.build();

		globalSetLayout = LxhDescriptorSetLayout::Builder(lxhDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		// set 1: albedo / normal / roughness / AO combined image samplers
		materialSetLayout = LxhDescriptorSetLayout::Builder(lxhDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		createDefaultTextures();
		createMaterials();
	}

	void App::run()
	{
		std::vector<std::unique_ptr<LxhBuffer>> uboBuffers(LxhSwapChain::MAX_FRAME_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<LxhBuffer>(
				lxhDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uboBuffers[i]->map();
		}

		std::vector<VkDescriptorSet> globalDescriptorSets(LxhSwapChain::MAX_FRAME_IN_FLIGHT);

		auto descriptorWrite = LxhDescriptorWriter(*globalSetLayout, *globalPool)
			.builds(globalDescriptorSets);

		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			VkDescriptorSet ds = globalDescriptorSets[i];
			descriptorWrite.writeBuffer(0, &bufferInfo);
			descriptorWrite.overwrite(ds);
			// reset the writer so writes don't accumulate across iterations
			descriptorWrite.clear();
		}

		RenderSystem pbrRenderSystem{
			lxhDevice,
			lxhRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout(),
			materialSetLayout->getDescriptorSetLayout() };
		PointLightSystem pointLightSystem{
			lxhDevice,
			lxhRenderer.getSwapChainRenderPass(),
			globalSetLayout->getDescriptorSetLayout() };
		LxhCamera camera{};

		auto viewerObject = LxhGameObject::createGameObject();
		viewerObject.transform.translation.z = -2.5f;
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!lxhWindow.ShouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime =
				std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			cameraController.moveInPlaneXZ(lxhWindow.GetGLFWWindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = lxhRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

			if (auto commandBuffer = lxhRenderer.beginFrame()) {
				int frameIndex = lxhRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					gameObjects };

				// update
				GlobalUbo ubo{};
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				ubo.inverseView = camera.getInverseView();
				pointLightSystem.update(frameInfo, ubo);
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// render
				lxhRenderer.beginSwapChainRenderpass(commandBuffer);

				// order here matters
				pbrRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);

				lxhRenderer.endSwapChainRenderpass(commandBuffer);
				lxhRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(lxhDevice.getDevice());

	}

	void App::createDefaultTextures()
	{
		// white = neutral albedo, full roughness and no occlusion;
		// rgb(128,128,255) decodes to the flat +Z tangent-space normal
		const std::vector<unsigned char> white = { 255, 255, 255, 255 };
		const std::vector<unsigned char> flatNormal = { 128, 128, 255, 255 };
		const VkExtent3D extent{ 1, 1, 1 };

		defaultAlbedo = std::make_shared<Texture2D>(lxhDevice, white, extent, VK_FORMAT_R8G8B8A8_SRGB);
		defaultNormal = std::make_shared<Texture2D>(lxhDevice, flatNormal, extent, VK_FORMAT_R8G8B8A8_UNORM);
		defaultRoughness = std::make_shared<Texture2D>(lxhDevice, white, extent, VK_FORMAT_R8G8B8A8_UNORM);
		defaultAO = std::make_shared<Texture2D>(lxhDevice, white, extent, VK_FORMAT_R8G8B8A8_UNORM);
	}

	void App::createMaterials()
	{
		for (auto& kv : gameObjects)
		{
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;

			for (const auto& mesh : obj.model->getMeshes())
			{
				if (mesh->material != nullptr) continue;

				std::shared_ptr<Texture2D> albedoTex;
				std::shared_ptr<Texture2D> normalTex;
				std::shared_ptr<Texture2D> roughnessTex;
				std::shared_ptr<Texture2D> aoTex;
				for (const auto& tex : mesh->m_textures) {
					if (tex.type == TextureType::Albedo && !albedoTex) albedoTex = tex.texture;
					else if (tex.type == TextureType::Normal && !normalTex) normalTex = tex.texture;
					else if (tex.type == TextureType::Roughness && !roughnessTex) roughnessTex = tex.texture;
					else if (tex.type == TextureType::AO && !aoTex) aoTex = tex.texture;
				}

				auto material = std::make_shared<LxhMaterial>(
					*globalPool,
					*materialSetLayout,
					albedoTex ? albedoTex : defaultAlbedo,
					normalTex ? normalTex : defaultNormal,
					roughnessTex ? roughnessTex : defaultRoughness,
					aoTex ? aoTex : defaultAO);

				MaterialParams params{};
				params.metallic = 0.1f;   // the backpack is mostly dielectric
				params.roughness = 1.0f;  // scale factor; the roughness map holds the base value
				params.ao = 1.0f;
				material->setParams(params);

				mesh->material = std::move(material);
			}
		}
	}

	void App::loadGameObjects()
	{
		std::shared_ptr<LxhModel> lveModel =
			LxhModel::createModelFromFile(lxhDevice, "assets/models/backpack/backpack.obj");
		auto flatVase = LxhGameObject::createGameObject();
		flatVase.model = lveModel;
		flatVase.transform.translation = { 0.f, 0.5f, 0.f };
		flatVase.transform.scale = { 0.2f, 0.2f, 0.2f };
		gameObjects.emplace(flatVase.getId(), std::move(flatVase));

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}  //
		};

		for (int i = 0; i < lightColors.size(); i++) {
			// PBR uses inverse-square attenuation, so the lights need a much
			// stronger intensity than the old Blinn-Phong demo used
			auto pointLight = LxhGameObject::makePointLight(10.f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
	}

	App::~App()
	{

	}

}
