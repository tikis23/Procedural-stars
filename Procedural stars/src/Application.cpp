#include "Application.h"

#include <iostream>
#include <string>
#include <format>

void ErrorCallback(int error, const char* msg) {
    std::cerr << " [" + std::to_string(error) + "] " + msg + '\n';
}

Application::Application()
{
}

Application::~Application()
{
    glfwSetErrorCallback(NULL);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_window->Handle());
    glfwTerminate();
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

    // init imgui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    ImGui_ImplGlfw_InitForOpenGL(m_window->Handle(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // create renderer
    m_renderer.reset(new Renderer);

	return true;
}

bool Application::Start()
{
    m_running = true;
    Camera cam({}, {0.0f, 400.0f, 0.0f}, {0.0f, 0.0f, 0.0f});
    while (m_running) {
        if (glfwWindowShouldClose(m_window->Handle()))
            m_running = false;
        float timerStart = glfwGetTime();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiIO& io = ImGui::GetIO();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode);
        if (ImGui::Begin("Settings", 0)) {
            if (ImGui::BeginTabBar("")) {
                if (ImGui::BeginTabItem("Info", 0, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoReorder))
                    ImGui::EndTabItem();
                if (ImGui::BeginTabItem("Camera", 0, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoReorder))
                    ImGui::EndTabItem();
                if (ImGui::BeginTabItem("Rendering", 0, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoReorder))
                    ImGui::EndTabItem();
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
        if (!m_window->IsIconified()) {

            cam.UpdateInput(m_window.get());
            cam.Update(m_window->GetAspectRatio());

            m_renderer->Draw(&cam, m_window.get());
        }
        if (ImGui::Begin("Settings", 0)) {
            if (ImGui::BeginTabBar("")) {
                if (ImGui::BeginTabItem("Info")) {
                    ImGui::Text("");
                    ImGui::Text(std::format("frame time: {} ms", glfwGetTime() - timerStart).c_str());
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        m_window->Update();
    }
	return true;
}
