#include "Renderer.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

Renderer::Renderer() {
	m_mainShader.reset(new Shader);
	m_mainShader->Load("res/shaders/main.vertex", GL_VERTEX_SHADER);
	m_mainShader->Load("res/shaders/main.fragment", GL_FRAGMENT_SHADER);
	m_mainShader->Link();
}

Renderer::~Renderer() {

}
struct Vertex {
    glm::vec3 pos;
};
void Renderer::Draw(Camera* cam) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_mainShader->Use();

    std::vector<Vertex>data;
    data.push_back({ {-1, -1, 0} });
    data.push_back({ {-1, 1, 0} });
    data.push_back({ {1, 1, 0} });
    data.push_back({ {1, 1, 0} });
    data.push_back({ {1, -1, 0} });
    data.push_back({ {-1, -1, 0} });
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(Vertex), &data[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);

    m_mainShader->uniformMatrix4("projection", glm::value_ptr(cam->GetProjection()));
    m_mainShader->uniformMatrix4("view", glm::value_ptr(cam->GetView()));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDeleteBuffers(1, &VBO);
    //// uniform variables
    //rasterShader->uniformMatrix4("projection", glm::value_ptr(camera->GetProjMat()));
    //rasterShader->uniformMatrix4("view", glm::value_ptr(camera->GetViewMat()));
    ////planet.model = glm::scale(glm::mat4(1), glm::vec3(10, 10, 10));
    //rasterShader->uniformMatrix4("model", glm::value_ptr(planet.model));
    //rasterShader->uniform3f("u_viewPos", camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);
    //rasterShader->uniform1i("smoothShading", GlobalVariable::smoothShading);
    //rasterShader->uniform1i("showNormals", GlobalVariable::showNormals);


    //for (int i = 0; i < 20; i++)
    //{
    //    int lod = 7;
    //    if (!planet.mesh[i]->data[5].empty())
    //    {
    //        float minDist = glm::length((glm::vec3)planet.mesh[i]->data[5][0].pos - camera->GetPosition());
    //        for (int l = 1; l < planet.mesh[i]->data[5].size(); l++)
    //        {
    //            float tempDist = glm::length((glm::vec3)planet.mesh[i]->data[5][l].pos - camera->GetPosition());
    //            if (tempDist < minDist)
    //                minDist = tempDist;
    //        }
    //        lod = (int)(glm::length(minDist) * 0.5 * 0.01);
    //    }
    //    if (lod < 0)
    //        lod = 0;
    //    if (lod > 5)
    //        lod = 5;
    //    planet.mesh[i]->Render(lod);
    //}

    //// skybox
    //if (GlobalVariable::wireframe)
    //{
    //    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //    skybox->Draw(camera);
    //    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //}
    //else
    //    skybox->Draw(camera);
}