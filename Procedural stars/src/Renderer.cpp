#include "Renderer.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "Planet.h"

Renderer::Renderer() {
    LoadShaders();
}

Renderer::~Renderer() {

}

void Renderer::LoadShaders() {
    m_mainShader.reset(new Shader);
    m_mainShader->Load("res/shaders/main.vertex", GL_VERTEX_SHADER);
    m_mainShader->Load("res/shaders/main.geometry", GL_GEOMETRY_SHADER);
    m_mainShader->Load("res/shaders/main.fragment", GL_FRAGMENT_SHADER);
    m_mainShader->Link();
}

void Renderer::Draw(Camera* cam) {
    // imgui settings
    if (ImGui::Begin("Settings")) {
        if (ImGui::BeginTabBar("")) {
            if (ImGui::BeginTabItem("Rendering")) {
                if (ImGui::Button("Reload shaders"))
                    LoadShaders();
                ImGui::Checkbox("Smooth Shading", &m_smoothShading);
                ImGui::Checkbox("Normals", &m_showNormals);
                ImGui::Checkbox("Wireframe", &m_showWireframe);
                ImGui::Checkbox("Back Face Culling", &m_backFaceCulling);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
 
    if (m_showWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (m_backFaceCulling)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
    glClearColor(0.1, 0.1, 0.1, 0);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // global uniforms
    m_mainShader->Use();
    m_mainShader->uniformMatrix4("projection", glm::value_ptr(cam->GetProjection()));
    m_mainShader->uniformMatrix4("view", glm::value_ptr(cam->GetView()));
    m_mainShader->uniform1i("smoothShading", m_smoothShading);
    m_mainShader->uniform1i("u_showNormals", m_showNormals);

    Planet planet(cam);
    // render each mesh
    std::vector<Mesh>& meshes = planet.GetMeshes();
    for (int i = 0; i < meshes.size(); i++) {
        m_mainShader->uniformMatrix4("model", glm::value_ptr(meshes[i].GetModelMatrix()));
        meshes[i].Draw();
    }
}