#pragma once
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <map>

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
		bool resizeable = false;
	};

	Window(WindowParameters params);
	~Window();
	void MakeContextCurrent();
	void Update();
	GLFWwindow* Handle() { return m_handle; };
	std::uint32_t GetWidth() { return m_parameters.windowWidth; };
	std::uint32_t GetHeight() { return m_parameters.windowHeight; };
	std::int32_t GetPositionX() { return m_parameters.windowPosX; };
	std::int32_t GetPositionY() { return m_parameters.windowPosY; };
	float GetAspectRatio() { return (float)m_parameters.windowWidth / m_parameters.windowHeight; };
private:
	static void FramebufferCallback(GLFWwindow* window, int width, int height);
	static void WindowPosCallback(GLFWwindow* window, int xpos, int ypos);

	GLFWwindow* m_handle;
	WindowParameters m_parameters;

	static std::map<GLFWwindow*, Window*> sm_windows;
};