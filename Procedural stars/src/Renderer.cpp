#include "Renderer.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "Planet.h"
#include "Buffer.h"

Renderer::Renderer() {
    LoadShaders();
}

Renderer::~Renderer() {

}

void Renderer::LoadShaders() {
    m_terrainShader.reset(new Shader);
    m_terrainShader->Load("res/shaders/terrain.vertex", GL_VERTEX_SHADER);
    m_terrainShader->Load("res/shaders/terrain.geometry", GL_GEOMETRY_SHADER);
    m_terrainShader->Load("res/shaders/terrain.fragment", GL_FRAGMENT_SHADER);
    m_terrainShader->Link();

    m_lightingShader.reset(new Shader);
    m_lightingShader->Load("res/shaders/lighting.vertex", GL_VERTEX_SHADER);
    m_lightingShader->Load("res/shaders/lighting.fragment", GL_FRAGMENT_SHADER);
    m_lightingShader->Link();

    m_debugShader.reset(new Shader);
    m_debugShader->Load("res/shaders/debug.vertex", GL_VERTEX_SHADER);
    m_debugShader->Load("res/shaders/debug.fragment", GL_FRAGMENT_SHADER);
    m_debugShader->Link();
}

void Renderer::Draw(Camera* cam, Window* window) {
    ///////////////// TEMP SOLUTION
    static GBuffer gbuffer(window->GetWidth(), window->GetHeight());
    gbuffer.BindWrite();
    // create planets (temporary solution)
    static std::vector<Planet> planets;
    static bool once = true;
    if (once) {
        planets.push_back({});
        once = false;
    }
    // temp solution
    static Mesh screenQuad;
    static bool initQuad = true;
    if (initQuad) {
        auto quad = screenQuad.GetVertexData();
        quad->push_back({ {-1, -1, 0} });
        quad->push_back({ {-1, 1, 0} });
        quad->push_back({ {1, 1, 0} });
        quad->push_back({ {-1, -1, 0} });
        quad->push_back({ {1, 1, 0} });
        quad->push_back({ {1, -1, 0} });
        screenQuad.Buffer();
        initQuad = false;
    }

    // imgui settings
    if (ImGui::Begin("Settings")) {
        if (ImGui::BeginTabBar("")) {
            if (ImGui::BeginTabItem("Rendering")) {
                if (ImGui::Button("Reload shaders"))
                    LoadShaders();
                ImGui::Checkbox("Smooth Shading", &m_smoothShading);
                ImGui::Checkbox("Wireframe", &m_showWireframe);
                ImGui::Checkbox("Back Face Culling", &m_backFaceCulling);
                ImGui::Checkbox("Debug mode", &m_debugMode);
                if (m_debugMode) {
                    if (ImGui::BeginChild("Debug settings", { 0, 0 }, true)) {
                        ImGui::Checkbox("Color", &debugvars.color);
                        ImGui::Checkbox("Normals", &debugvars.normals);
                        ImGui::Checkbox("Diffuse", &debugvars.diffuse);
                        ImGui::Checkbox("Specular", &debugvars.specular);
                    }
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Info")) {
                ImGui::Text(("\n\n\nTriangles drawn: "+std::to_string(Mesh::TriangleCount)).c_str());
                Mesh::TriangleCount = 0;
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
    for (int i = 0; i < planets.size(); i++) {
        planets[i].Update();
        // render terrain
        m_terrainShader->Use();

        // geometry uniforms
        m_terrainShader->uniformMatrix4("projection", glm::value_ptr(cam->GetProjection()));
        m_terrainShader->uniformMatrix4("view", glm::value_ptr(cam->GetView()));
        m_terrainShader->uniformMatrix4("model", glm::value_ptr(planets[i].GetPositionModelMatrix()));
        m_terrainShader->uniform1i("smoothShading", m_smoothShading);

        planets[i].Render(cam->GetPosition());
    }

    // select shader
    auto defferedShader = m_lightingShader.get();
    if (m_debugMode)
        defferedShader = m_debugShader.get();

    // deffered shading
    defferedShader->Use();
    gbuffer.BindRead(defferedShader);
    defferedShader->uniform3f("u_cameraPos", cam->GetPosition());

    // debug mode uniforms
    if (m_debugMode) {
        int dbgv[4] = {
            debugvars.color,
            debugvars.normals,
            debugvars.diffuse,
            debugvars.specular
        };
        defferedShader->uniformArri("u_debug", 4, dbgv);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glClearColor(0.1, 0.1, 0.1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    screenQuad.Draw();
}