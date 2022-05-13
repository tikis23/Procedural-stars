#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "Window.h"
#include "Renderer.h"

class Application {
public:
	Application();
	~Application();
	bool Init();
	bool Start();
private:
	bool m_running = false;
	std::shared_ptr<Window> m_window;
	std::shared_ptr<Renderer> m_renderer;
};