#include "Window.h"

Window::Window(WindowParameters params) {
	m_parameters = params;
	m_WindowedWindowWidth = m_parameters.windowWidth;
	m_WindowedWindowHeight = m_parameters.windowHeight;
	m_WindowedWindowPosX = m_parameters.windowPosX;
	m_WindowedWindowPosY = m_parameters.windowPosY;

	const GLFWvidmode* monitorMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RESIZABLE, m_parameters.resizeable);

	if (m_parameters.mode == WINDOW_MODE::FULLSCREEN) {
		m_handle = glfwCreateWindow(m_parameters.windowWidth, m_parameters.windowHeight, m_parameters.title.c_str(), glfwGetPrimaryMonitor(), NULL);
		m_parameters.windowWidth = monitorMode->width;
		m_parameters.windowHeight = monitorMode->height;
		m_parameters.windowPosX = 0;
		m_parameters.windowPosY = 0;
	}
	else if (m_parameters.mode == WINDOW_MODE::BORDERLESS) {
		m_handle = glfwCreateWindow(monitorMode->width, monitorMode->height, m_parameters.title.c_str(), NULL, NULL);
		m_parameters.windowWidth = monitorMode->width;
		m_parameters.windowHeight = monitorMode->height;
		m_parameters.windowPosX = 0;
		m_parameters.windowPosY = 0;
	}
	else if (m_parameters.mode == WINDOW_MODE::WINDOWED) {
		m_handle = glfwCreateWindow(m_parameters.windowWidth, m_parameters.windowHeight, m_parameters.title.c_str(), NULL, NULL);
	}
	glfwSetWindowPos(m_handle, m_parameters.windowPosX, m_parameters.windowPosY);
	glfwSetFramebufferSizeCallback(m_handle, FramebufferCallback);
	glfwSetWindowPosCallback(m_handle, WindowPosCallback);
	glfwSetWindowIconifyCallback(m_handle, WindowIconifyCallback);
	Init(m_handle);
	if (m_parameters.iconified)
		glfwIconifyWindow(m_handle);
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
	for (int i = 0; i < win->m_resizeCallbacks.size(); i++) {
		win->m_resizeCallbacks[i](width, height);
	}
}

void Window::WindowPosCallback(GLFWwindow* window, int xpos, int ypos) {
	Window* win = sm_windows.at(window);
	if (win == nullptr)
		return;
	win->m_parameters.windowPosX = xpos;
	win->m_parameters.windowPosY = ypos;
}

void Window::WindowIconifyCallback(GLFWwindow* window, int iconified) {
	Window* win = sm_windows.at(window);
	if (win == nullptr)
		return;
	win->m_parameters.iconified = iconified;
}

void Window::AddResizeCallback(std::function<void(std::uint32_t WinSizeX, std::uint32_t WinSizeY)> callback) {
	m_resizeCallbacks.push_back(callback);
}

void Window::ChangeMode(WINDOW_MODE newmode) {
	const GLFWvidmode* monitorMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	if (m_parameters.mode != WINDOW_MODE::FULLSCREEN && newmode == WINDOW_MODE::FULLSCREEN) {
		if (m_parameters.mode == WINDOW_MODE::WINDOWED) {
			m_WindowedWindowWidth = m_parameters.windowWidth;
			m_WindowedWindowHeight = m_parameters.windowHeight;
			m_WindowedWindowPosX = m_parameters.windowPosX;
			m_WindowedWindowPosY = m_parameters.windowPosY;
		}
		m_parameters.mode = WINDOW_MODE::FULLSCREEN;
		m_parameters.windowWidth = monitorMode->width;
		m_parameters.windowHeight = monitorMode->height;
		m_parameters.windowPosX = 0;
		m_parameters.windowPosY = 0;
		glfwSetWindowMonitor(m_handle, glfwGetPrimaryMonitor(), 0, 0, monitorMode->width, monitorMode->height, GLFW_DONT_CARE);
	}
	else if (m_parameters.mode != WINDOW_MODE::BORDERLESS && newmode == WINDOW_MODE::BORDERLESS) {
		if (m_parameters.mode == WINDOW_MODE::WINDOWED) {
			m_WindowedWindowWidth = m_parameters.windowWidth;
			m_WindowedWindowHeight = m_parameters.windowHeight;
			m_WindowedWindowPosX = m_parameters.windowPosX;
			m_WindowedWindowPosY = m_parameters.windowPosY;
		}
		m_parameters.mode = WINDOW_MODE::BORDERLESS;
		m_parameters.windowWidth = monitorMode->width;
		m_parameters.windowHeight = monitorMode->height;
		m_parameters.windowPosX = 0;
		m_parameters.windowPosY = 0;
		glfwSetWindowMonitor(m_handle, NULL, 0, 0, monitorMode->width, monitorMode->height, GLFW_DONT_CARE);
	}
	else if (m_parameters.mode != WINDOW_MODE::WINDOWED && newmode == WINDOW_MODE::WINDOWED) {
		m_parameters.mode = WINDOW_MODE::WINDOWED;
		m_parameters.windowWidth = m_WindowedWindowWidth;
		m_parameters.windowHeight = m_WindowedWindowHeight;
		m_parameters.windowPosX = m_WindowedWindowPosX;
		m_parameters.windowPosY = m_WindowedWindowPosY;
		glfwSetWindowMonitor(m_handle, NULL, m_parameters.windowPosX, m_parameters.windowPosY, m_parameters.windowWidth, m_parameters.windowHeight, GLFW_DONT_CARE);
	}
}

void Window::ChangeMode(WINDOW_MODE newmode, std::uint32_t windowWidth, std::uint32_t windowHeight) {
}

std::map<GLFWwindow*, Window*> Window::sm_windows;