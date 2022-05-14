#include "Window.h"

Window::Window(WindowParameters params) {
	m_parameters = params;
	const GLFWvidmode* monitorMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	glfwWindowHint(GLFW_RESIZABLE, m_parameters.resizeable);

	if (m_parameters.mode == WINDOW_MODE::FULLSCREEN)
		m_handle = glfwCreateWindow(m_parameters.windowWidth, m_parameters.windowHeight, m_parameters.title.c_str(), glfwGetPrimaryMonitor(), NULL);
	else if (m_parameters.mode == WINDOW_MODE::BORDERLESS) {
		m_handle = glfwCreateWindow(monitorMode->width, monitorMode->height, m_parameters.title.c_str(), glfwGetPrimaryMonitor(), NULL);
		m_parameters.windowWidth = monitorMode->width;
		m_parameters.windowHeight = monitorMode->height;
	}
	else if (m_parameters.mode == WINDOW_MODE::WINDOWED)
		m_handle = glfwCreateWindow(m_parameters.windowWidth, m_parameters.windowHeight, m_parameters.title.c_str(), NULL, NULL);
	glfwSetWindowPos(m_handle, m_parameters.windowPosX, m_parameters.windowPosY);
	glfwSetFramebufferSizeCallback(m_handle, FramebufferCallback);
	glfwSetWindowPosCallback(m_handle, WindowPosCallback);
	Init(m_handle);
	sm_windows[m_handle] = this;
}
Window::~Window() {
	glfwDestroyWindow(m_handle);
	sm_windows.erase(m_handle);
}

void Window::MakeContextCurrent() {
	glfwMakeContextCurrent(m_handle);
}

void Window::Update() {
	glfwSwapBuffers(m_handle);
	Poll();
	glfwPollEvents();
}

void Window::FramebufferCallback(GLFWwindow* window, int width, int height) {
	glfwMakeContextCurrent(window);
	glViewport(0, 0, width, height);
	Window* win = sm_windows.at(window);
	if (win == nullptr)
		return;
	win->m_parameters.windowWidth = width;
	win->m_parameters.windowHeight = height;
}

void Window::WindowPosCallback(GLFWwindow* window, int xpos, int ypos) {
	Window* win = sm_windows.at(window);
	if (win == nullptr)
		return;
	win->m_parameters.windowPosX = xpos;
	win->m_parameters.windowPosY = ypos;
}

std::map<GLFWwindow*, Window*> Window::sm_windows;