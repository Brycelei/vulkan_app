#include<app.h>

#include "keyboard_movement_controller.h"
#include "lxh_buffer.h"
#include "lxh_camera.h"
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
		globalPool = LxhDescriptorPool::Builder(lxhDevice)
			.setMaxSets(LxhSwapChain::MAX_FRAME_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LxhSwapChain::MAX_FRAME_IN_FLIGHT)
			.build();
		loadGameObjects();
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

		auto globalSetLayout =
			LxhDescriptorSetLayout::Builder(lxhDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(LxhSwapChain::MAX_FRAME_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			LxhDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

	  RenderSystem simpleRenderSystem{
	  lxhDevice,
	  lxhRenderer.getSwapChainRenderPass(),
	  globalSetLayout->getDescriptorSetLayout() };
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
				simpleRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);

				lxhRenderer.endSwapChainRenderpass(commandBuffer);
				lxhRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(lxhDevice.getDevice());

	}

	void App::loadGameObjects()
	{
		std::shared_ptr<LxhModel> lveModel =
			LxhModel::createModelFromFile(lxhDevice, "assets/models/fx11/FX11_A2.obj");
		auto flatVase = LxhGameObject::createGameObject();
		flatVase.model = lveModel;
		flatVase.transform.translation = { -.5f, .5f, 0.f };
		flatVase.transform.scale = { 0.02f, 0.02f, 0.02f };
		gameObjects.emplace(flatVase.getId(), std::move(flatVase));

		lveModel = LxhModel::createModelFromFile(lxhDevice, "assets/models/smooth_vase.obj");
		auto smoothVase = LxhGameObject::createGameObject();
		smoothVase.model = lveModel;
		smoothVase.transform.translation = { .5f, .5f, 0.f };
		smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

		lveModel = LxhModel::createModelFromFile(lxhDevice, "assets/models/quad.obj");
		auto floor = LxhGameObject::createGameObject();
		floor.model = lveModel;
		floor.transform.translation = { 0.f, .5f, 0.f };
		floor.transform.scale = { 3.f, 1.f, 3.f };
		gameObjects.emplace(floor.getId(), std::move(floor));

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}  //
		};

		for (int i = 0; i < lightColors.size(); i++) {
			auto pointLight = LxhGameObject::makePointLight(0.2f);
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