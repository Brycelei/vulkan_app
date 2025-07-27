#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include<string>

namespace lxh {
class lxhWindow
{
public:
	lxhWindow(int w, int h, std::string name);
	~lxhWindow();

	lxhWindow(const lxhWindow&) = delete;
	lxhWindow& operator=(const lxhWindow&) = delete;
	lxhWindow(lxhWindow&&) = delete;
	lxhWindow& operator=(lxhWindow&&) = delete;

	bool ShouldClose() {
		return glfwWindowShouldClose(window);
	};
	VkExtent2D GetExtent() const {
		return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	}
	bool wasWindowResized() const {
		return framebufferResized;
	}
	void resetWindowResizedFlag() {
		framebufferResized = false;
	}

	GLFWwindow* GetGLFWWindow() const {
		return window;
	}

	void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
	void InitWindow();

	int width;
	int height;
	bool framebufferResized = false;

	std::string windowName;
	GLFWwindow* window;
};
}