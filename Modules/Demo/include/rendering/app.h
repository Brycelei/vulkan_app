#pragma once

#include "lxh_descriptors.h"
#include "lxh_device.h"
#include "lxh_game_object.h"
#include "lxh_renderer.h"
#include "lxh_application.h"
#include "lxh_texture.h"

// std
#include <memory>
#include <vector>

namespace lxh {

	class App {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		App();
		~App();

		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void run();

	private:
		void loadGameObjects();
		void createDefaultTextures();
		void createMaterials();

		LxhWindow lxhWindow{ WIDTH, HEIGHT, "Vulkan Tutorial" };
		LxhDevice& lxhDevice = LxhDevice::getInstance(lxhWindow); // 单例获取
		LxhRenderer lxhRenderer{ lxhWindow, lxhDevice };

		// note: order of declarations matters
		std::unique_ptr<LxhDescriptorPool> globalPool{};
		std::unique_ptr<LxhDescriptorSetLayout> globalSetLayout{};
		std::unique_ptr<LxhDescriptorSetLayout> materialSetLayout{};

		// 1x1 fallback textures shared by meshes without a given PBR slot
		std::shared_ptr<Texture2D> defaultAlbedo;
		std::shared_ptr<Texture2D> defaultNormal;
		std::shared_ptr<Texture2D> defaultRoughness;
		std::shared_ptr<Texture2D> defaultAO;

		LxhGameObject::Map gameObjects;
	};
}
