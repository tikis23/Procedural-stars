#include "Application.h"

#include <iostream>
#include <string>

void ErrorCallback(int error, const char* msg) {
    std::cerr << " [" + std::to_string(error) + "] " + msg + '\n';
}

Application::Application()
{
}

Application::~Application()
{
}

bool Application::Init()
{
    std::cout << glfwGetVersionString() << "\n";
    glfwSetErrorCallback(ErrorCallback);
    // init glfw
    if (!glfwInit()) {
        ErrorCallback(-1, "Could not initialize GLFW");
        return false;
    }

    // create window
    Window::WindowParameters windowParams;
    windowParams.mode = Window::WINDOW_MODE::WINDOWED;
    windowParams.title = "Procedural stars";
    windowParams.windowWidth = 1000;
    windowParams.windowHeight = 800;
    windowParams.windowPosX = 400;
    windowParams.windowPosY = 200;
    windowParams.resizeable = false;
    m_window.reset(new Window(windowParams));
    m_window->MakeContextCurrent();

    // init glew
    if (glewInit() != GLEW_OK) {
        ErrorCallback(-1, "Could not initialize GLEW");
        return false;
    }

    // create renderer
    m_renderer.reset(new Renderer);

	return true;
}

bool Application::Start()
{
    m_running = true;
        Camera cam({}, m_window->GetAspectRatio(), {0.0f, 0.0f, 2.0f}, {90.0f, 0.0f, 0.0f});
    while (m_running) {
        if (glfwWindowShouldClose(m_window->Handle()))
            m_running = false;
        cam.UpdateInput(m_window.get());
        cam.Update();
        m_renderer->Draw(&cam);
        m_window->Update();
    }
	return true;
}
