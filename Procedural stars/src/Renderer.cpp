#include "Renderer.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "Planet.h"
#include "Timer.h"
#include "Debug.h"
#include "NoiseMap.h"

Renderer::Renderer(Window* window) :
    m_gbuffer(window->GetWidth(), window->GetHeight()),
    m_ssaobuffer(window->GetWidth(), window->GetHeight()) {
    // set patch vertices
    GLint MaxPatchVertices = 0;
    glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
    printf("Max supported patch vertices %d\n", MaxPatchVertices);
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    LoadShaders();

    // attach buffers to window resize callback
    window->AddResizeCallback(std::bind(&GBuffer::ResizeCallback, m_gbuffer, std::placeholders::_1, std::placeholders::_2));
    window->AddResizeCallback(std::bind(&SSAOBuffer::ResizeCallback, m_ssaobuffer, std::placeholders::_1, std::placeholders::_2));

    Debug::Init();
}

Renderer::~Renderer() {
    Debug::Exit();
}

void Renderer::LoadShaders() {
    Timer time;
    m_terrainShader.reset(new Shader);
    m_terrainShader->Load("res/shaders/terrain.vertex", GL_VERTEX_SHADER);
    m_terrainShader->Load("res/shaders/terrain.tesscontrol", GL_TESS_CONTROL_SHADER);
    m_terrainShader->Load("res/shaders/terrain.tesseval", GL_TESS_EVALUATION_SHADER);
    m_terrainShader->Load("res/shaders/terrain.fragment", GL_FRAGMENT_SHADER);
    m_terrainShader->Link();

    m_SSAOShader.reset(new Shader);
    m_SSAOShader->Load("res/shaders/ssao.vertex", GL_VERTEX_SHADER);
    m_SSAOShader->Load("res/shaders/ssao.fragment", GL_FRAGMENT_SHADER);
    m_SSAOShader->Link();

    m_lightingShader.reset(new Shader);
    m_lightingShader->Load("res/shaders/lighting.vertex", GL_VERTEX_SHADER);
    m_lightingShader->Load("res/shaders/lighting.fragment", GL_FRAGMENT_SHADER);
    m_lightingShader->Link();


    std::cout << std::format("Shaders loaded. ({} ms)\n", time.Elapsed());
}

void Renderer::Draw(Camera* cam, Window* window) {
    ///////////////// TEMP SOLUTION
    static std::vector<Planet> planets;
    static bool once = true;
    static Mesh screenQuad;
    if (once) {
        planets.push_back({});
        auto quad = screenQuad.GetVertexData();
        quad->push_back({ {-1, -1, 0} });
        quad->push_back({ {-1, 1, 0} });
        quad->push_back({ {1, 1, 0} });
        quad->push_back({ {-1, -1, 0} });
        quad->push_back({ {1, 1, 0} });
        quad->push_back({ {1, -1, 0} });
        screenQuad.Buffer();
        once = false;
        m_gbuffer.ResizeCallback(window->GetWidth(), window->GetHeight());
        m_ssaobuffer.ResizeCallback(window->GetWidth(), window->GetHeight());
    }

    // imgui settings
    if (ImGui::Begin("Settings")) {
        if (ImGui::BeginTabBar("")) {
            if (ImGui::BeginTabItem("Rendering")) {
                if (ImGui::Button("Windowed"))
                    window->ChangeMode(Window::WINDOW_MODE::WINDOWED);
                ImGui::SameLine();
                if (ImGui::Button("Borderless"))
                    window->ChangeMode(Window::WINDOW_MODE::BORDERLESS);
                ImGui::SameLine();
                if (ImGui::Button("Fullscreen"))
                    window->ChangeMode(Window::WINDOW_MODE::FULLSCREEN);
                if (ImGui::Button("Reload shaders"))
                    LoadShaders();
                ImGui::Checkbox("Wireframe", &m_showWireframe);
                ImGui::Checkbox("Show LOD", &m_showLod);
                ImGui::Checkbox("SSAO", &m_ssao);
                ImGui::Checkbox("Back Face Culling", &m_backFaceCulling);
                bool dbg = Debug::GeometryEnabled();
                ImGui::Checkbox("Debug geometry", &dbg);
                Debug::GeometryEnable(dbg);
                dbg = Debug::LightingEnabled();
                ImGui::Checkbox("Debug lighting", &dbg);
                Debug::LightingEnable(dbg);
                if (Debug::LightingEnabled()) {
                    if (ImGui::BeginChild("Debug settings", { 0, 0 }, true)) {
                        ImGui::Checkbox("Color", &debugvars.color);
                        ImGui::Checkbox("Normals", &debugvars.normals);
                        ImGui::Checkbox("Diffuse", &debugvars.diffuse);
                        ImGui::Checkbox("Specular", &debugvars.specular);
                        ImGui::Checkbox("SSAO", &debugvars.ssao);
                    }
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Info")) {
                ImGui::Text(("\n\n\n\nVertex count: " + std::to_string(Mesh::VertexCount)).c_str());
                Mesh::VertexCount = 0;
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    
    // debug settings
    glClearColor(0.1, 0.1, 0.1, 0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_showWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (m_backFaceCulling)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);


    // render each planet
    m_gbuffer.BindWrite();
    m_terrainShader->Use();
    m_terrainShader->uniformMatrix4("projection", glm::value_ptr(cam->GetProjection()));
    m_terrainShader->uniformMatrix4("view", glm::value_ptr(cam->GetView()));
    m_terrainShader->uniform3f("u_cameraPos", cam->GetPosition());
    m_terrainShader->uniform1i("u_showLod", m_showLod);
    static NoiseMap noise(20, 20);

    for (int i = 0; i < planets.size(); i++) {
        planets[i].Update();
        m_terrainShader->uniformMatrix4("model", glm::value_ptr(planets[i].GetPositionModelMatrix()));
        noise.BindNoise(m_terrainShader.get(), "u_noise", 0);
        planets[i].Render(cam->GetPosition(), m_terrainShader.get());
    }

    // debug
    Debug::Render(glm::value_ptr(cam->GetProjection()), glm::value_ptr(cam->GetView()));

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glClearColor(0.1, 0.1, 0.1, 0);

    // SSAO pass
    m_SSAOShader->Use();
    m_ssaobuffer.BindWrite();
    m_gbuffer.BindPosition(m_SSAOShader.get(), 0);
    m_gbuffer.BindNormal(m_SSAOShader.get(), 1);
    m_ssaobuffer.BindNoise(m_SSAOShader.get(), 2);
    m_ssaobuffer.BindKernel(m_SSAOShader.get());
    m_SSAOShader->uniformMatrix4("u_projection", glm::value_ptr(cam->GetProjection()));
    m_SSAOShader->uniform2f("u_screen", window->GetWidth(), window->GetHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    screenQuad.Draw(GL_TRIANGLES);

    // light pass
    auto defferedShader = m_lightingShader.get();
    if (Debug::LightingEnabled())
        defferedShader = Debug::GetLightingShader();
    defferedShader->Use();
    m_gbuffer.BindRead();
    m_gbuffer.BindColor(defferedShader, 0);
    m_gbuffer.BindPosition(defferedShader, 1);
    m_gbuffer.BindNormal(defferedShader, 2);
    m_ssaobuffer.BindSSAO(defferedShader, 3);
    defferedShader->uniform3f("u_cameraPos", cam->GetPosition());
    defferedShader->uniform1i("u_ssao", m_ssao);

    // debug mode uniforms
    if (Debug::LightingEnabled()) {
        int dbgv[] = {
            debugvars.color,
            debugvars.normals,
            debugvars.diffuse,
            debugvars.specular,
            debugvars.ssao
        };
        defferedShader->uniformArr1i("u_debug", 5, dbgv);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    screenQuad.Draw(GL_TRIANGLES);
}