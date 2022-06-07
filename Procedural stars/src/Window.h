#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <functional>

#include "Input.h"

class Window : public Input {
public:
	enum class WINDOW_MODE {
		WINDOWED,
		FULLSCREEN,
		BORDERLESS
	};
	struct WindowParameters {
		WINDOW_MODE mode = WINDOW_MODE::WINDOWED;
		std::string title = "This is a title";
		std::uint32_t windowWidth = 500;
		std::uint32_t windowHeight = 400;
		std::int32_t windowPosX = 100;
		std::int32_t windowPosY = 100;
		bool iconified = false;
		bool resizeable = false;
	};

	Window(WindowParameters params);
	~Window();
	void MakeContextCurrent();
	void Update();
	void AddResizeCallback(std::function<void(std::uint32_t WinSizeX, std::uint32_t WinSizeY)>);
	void ChangeMode(WINDOW_MODE newmode);
	void ChangeMode(WINDOW_MODE newmode, std::uint32_t windowWidth, std::uint32_t windowHeight);
	GLFWwindow* Handle() { return m_handle; };
	std::uint32_t GetWidth() { return m_parameters.windowWidth; };
	std::uint32_t GetHeight() { return m_parameters.windowHeight; };
	std::int32_t GetPositionX() { return m_parameters.windowPosX; };
	std::int32_t GetPositionY() { return m_parameters.windowPosY; };
	float GetAspectRatio() { return (float)m_parameters.windowWidth / m_parameters.windowHeight; };
	bool IsIconified() { return m_parameters.iconified; };
private:
	static void FramebufferCallback(GLFWwindow* window, int width, int height);
	static void WindowPosCallback(GLFWwindow* window, int xpos, int ypos);
	static void WindowIconifyCallback(GLFWwindow* window, int iconified);

	std::uint32_t m_WindowedWindowWidth;
	std::uint32_t m_WindowedWindowHeight;
	std::int32_t  m_WindowedWindowPosX;
	std::int32_t  m_WindowedWindowPosY;

	GLFWwindow* m_handle;
	WindowParameters m_parameters;
	std::vector<std::function<void(std::uint32_t WinSizeX, std::uint32_t WinSizeY)>> m_resizeCallbacks;
	static std::map<GLFWwindow*, Window*> sm_windows;
};