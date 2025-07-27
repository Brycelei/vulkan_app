#include<lxh_application.h>
#include <stdexcept>
namespace lxh {

	lxhWindow::lxhWindow(int w, int h, std::string name)
	{
		InitWindow();
	}
	lxhWindow::~lxhWindow()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void lxhWindow::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

		if (!window) {
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

	}
	void lxhWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}

	void lxhWindow::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
	{
		auto lveWindow = reinterpret_cast<lxhWindow*>(glfwGetWindowUserPointer(window));
		lveWindow->framebufferResized = true;
		lveWindow->width = width;
		lveWindow->height = height;
	}
}