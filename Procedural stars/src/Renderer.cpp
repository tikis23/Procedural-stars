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
    m_terrainShader.reset(new Shader);
    m_terrainShader->Load("res/shaders/terrain.vertex", GL_VERTEX_SHADER);
    m_terrainShader->Load("res/shaders/terrain.geometry", GL_GEOMETRY_SHADER);
    m_terrainShader->Load("res/shaders/terrain.fragment", GL_FRAGMENT_SHADER);
    m_terrainShader->Link();

    m_oceanShader.reset(new Shader);
    m_oceanShader->Load("res/shaders/ocean.vertex", GL_VERTEX_SHADER);
    m_oceanShader->Load("res/shaders/ocean.geometry", GL_GEOMETRY_SHADER);
    m_oceanShader->Load("res/shaders/ocean.fragment", GL_FRAGMENT_SHADER);
    m_oceanShader->Link();
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

    // create planets (temporary solution)
    static std::vector<Planet> planets;
    static bool once = true;
    if (once) {
        planets.push_back({});
        planets.back().SetPosition({ -800, 0, 0 });
        planets.push_back({});
        planets.back().SetPosition({ 800, 100, 0 });
        once = false;
    }

    // render each planet
    for (int i = 0; i < planets.size(); i++) {
        Planet* planet = &planets[i];
        planet->Update();
        std::vector<Mesh*> meshes = planet->GetMeshes();


        // calculate rotation, lod, z offset      
        // z offset
        float r = planet->GetRadius();
        float R = planet->GetRadiusMax();
        float d = glm::length(cam->GetPosition() - planet->GetPosition());
        float h = sqrtf(d * d - r * r);
        float s = sqrtf(R * R - r * r);
        float zs = (R * R + d * d - (h + s) * (h + s)) / (2 * r * (h + s));

        // face camera
        glm::vec3 b = glm::normalize(cam->GetPosition() + glm::vec3{ 1, 0, 0 });
        b = { 0, 0, 1 };
        glm::vec3 w = glm::normalize(cam->GetPosition() - planet->GetPosition());
        glm::vec3 v = glm::normalize(glm::cross(w, b));
        glm::vec3 u = glm::cross(w, v);

        // lod
        int lod = log2((d) * (0.02 + sqrt(sqrt(d)) * 0.0015)) * 2;
        if (lod > 15)
            lod = 15;
        if (lod < 0)
            lod = 0;
        

        // render terrain
        m_terrainShader->Use();
        m_terrainShader->uniformMatrix4("projection", glm::value_ptr(cam->GetProjection()));
        m_terrainShader->uniformMatrix4("view", glm::value_ptr(cam->GetView()));
        m_terrainShader->uniform1i("smoothShading", m_smoothShading);
        m_terrainShader->uniform1i("u_showNormals", m_showNormals);
        m_terrainShader->uniform3f("u_cameraPos", cam->GetPosition());

        m_terrainShader->uniform1f("u_zOffset", zs);
        m_terrainShader->uniform1f("u_r", r);
        m_terrainShader->uniform3f("u_u", u);
        m_terrainShader->uniform3f("u_v", v);
        m_terrainShader->uniform3f("u_w", w);
        m_terrainShader->uniformMatrix4("u_modelRot", glm::value_ptr(planet->GetRotationModelMatrix()));
        m_terrainShader->uniformMatrix4("model", glm::value_ptr(planet->GetPositionModelMatrix()));

        meshes[lod]->Draw();

        // render ocean
        m_oceanShader->Use();
        m_oceanShader->uniformMatrix4("projection", glm::value_ptr(cam->GetProjection()));
        m_oceanShader->uniformMatrix4("view", glm::value_ptr(cam->GetView()));
        m_oceanShader->uniform1i("smoothShading", m_smoothShading);
        m_oceanShader->uniform1i("u_showNormals", m_showNormals);
        m_oceanShader->uniform3f("u_cameraPos", cam->GetPosition());

        m_oceanShader->uniform1f("u_zOffset", zs);
        m_oceanShader->uniform1f("u_r", r);
        m_oceanShader->uniform3f("u_u", u);
        m_oceanShader->uniform3f("u_v", v);
        m_oceanShader->uniform3f("u_w", w);
        m_oceanShader->uniformMatrix4("u_modelRot", glm::value_ptr(planet->GetRotationModelMatrix()));
        m_oceanShader->uniformMatrix4("model", glm::value_ptr(planet->GetPositionModelMatrix()));

        meshes[lod]->Draw();
    }
}