#include<lxh_application.h>
#include <stdexcept>
namespace lxh {

	LxhWindow::LxhWindow(int w, int h, std::string name):width{w}, height{h}, windowName{name}
	{
		InitWindow();
	}
	LxhWindow::~LxhWindow()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void LxhWindow::InitWindow()
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
	void LxhWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}

	void LxhWindow::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
	{
		auto lveWindow = reinterpret_cast<LxhWindow*>(glfwGetWindowUserPointer(window));
		lveWindow->framebufferResized = true;
		lveWindow->width = width;
		lveWindow->height = height;
	}
}