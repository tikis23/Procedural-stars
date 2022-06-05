#include "Debug.h"

void Debug::Init() {
    m_geometryShader.reset(new Shader);
    m_geometryShader->Load("res/shaders/debuggeometry.vertex", GL_VERTEX_SHADER);
    m_geometryShader->Load("res/shaders/debuggeometry.fragment", GL_FRAGMENT_SHADER);
    m_geometryShader->Link();

    m_lightingShader.reset(new Shader);
    m_lightingShader->Load("res/shaders/debuglighting.vertex", GL_VERTEX_SHADER);
    m_lightingShader->Load("res/shaders/debuglighting.fragment", GL_FRAGMENT_SHADER);
    m_lightingShader->Link();

    glGenBuffers(1, &m_VBO);
    glGenVertexArrays(1, &m_VAO);
}

void Debug::Exit() {
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_VAO);
}

void Debug::Render(float* projection, float* view) {
    if (!m_geometryEnabled || m_worldGeometry.empty())
        return;

    m_geometryShader->Use();
    m_geometryShader->uniformMatrix4("u_projection", projection);
    m_geometryShader->uniformMatrix4("u_view", view);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glBufferData(GL_ARRAY_BUFFER, m_worldGeometry.size() * sizeof(DebugPoint), &m_worldGeometry[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(DebugPoint), (void*)offsetof(DebugPoint, point));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(DebugPoint), (void*)offsetof(DebugPoint, color));
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_LINES, 0, m_worldGeometry.size());
    m_worldGeometry.clear();
}

void Debug::DrawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color) {
    if (!m_geometryEnabled)
        return;
    m_worldGeometry.push_back({ p1, color });
    m_worldGeometry.push_back({ p2, color });
}

bool Debug::m_lightingEnabled = false;
bool Debug::m_geometryEnabled = false;
unsigned int Debug::m_VBO;
unsigned int Debug::m_VAO;
std::shared_ptr<Shader> Debug::m_geometryShader;
std::shared_ptr<Shader> Debug::m_lightingShader;
std::vector<Debug::DebugPoint> Debug::m_worldGeometry;