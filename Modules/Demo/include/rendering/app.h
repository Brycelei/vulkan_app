#pragma once

#include "lxh_descriptors.h"
#include "lxh_device.h"
#include "lxh_game_object.h"
#include "lxh_renderer.h"
#include "lxh_application.h"

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

		LxhWindow lxhWindow{ WIDTH, HEIGHT, "Vulkan Tutorial" };
		LxhDevice lxhDevice{ lxhWindow };
		LxhRenderer lxhRenderer{ lxhWindow, lxhDevice };

		// note: order of declarations matters
		std::unique_ptr<LxhDescriptorPool> globalPool{};
		LxhGameObject::Map gameObjects;
	};
}
