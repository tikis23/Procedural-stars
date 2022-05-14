#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "imgui/imgui.h"
#include <format>
Camera::Camera(CameraParameters params, glm::vec3 position, glm::vec3 rotation) {
    SetParameters(params);
    m_position = position;
    m_rotation = rotation;
    if (m_rotation.y > 89.0f)
        m_rotation.y = 89.0f;
    if (m_rotation.y < -89.0f)
        m_rotation.y = -89.0f;
    m_rotation.x = fmodf(m_rotation.x, 360.0f);
    m_rotation.z = fmodf(m_rotation.z, 360.0f);
    // calculate front && up vectors
    m_up = { 0, 1, 0 };
    m_front.x = cosf(glm::radians(m_rotation.x)) * cosf(glm::radians(m_rotation.y));
    m_front.y = sinf(glm::radians(m_rotation.y));
    m_front.z = sinf(glm::radians(m_rotation.x)) * cosf(glm::radians(m_rotation.y));
    m_front = glm::normalize(m_front);
}

Camera::~Camera() {
}

void Camera::UpdateInput(Window* window) {
    float time = glfwGetTime();
    float dt = time - m_previousTime;
    m_previousTime = time;

    // rotation
    glm::vec2 mousePos = window->GetMousePosition();
    if (window->IsHeld(Input::MOUSE::RIGHT)) {
        glm::vec2 offset = mousePos - m_previousMousePosition;
        m_rotation.x += offset.x * m_lookSpeed * dt;
        m_rotation.y += offset.y * m_lookSpeed * dt;
        if (m_rotation.y > 89.0f)
            m_rotation.y = 89.0f;
        if (m_rotation.y < -89.0f)
            m_rotation.y = -89.0f;
        m_rotation.x = fmodf(m_rotation.x, 360.0f);
        m_rotation.z = fmodf(m_rotation.z, 360.0f);

        // calculate front && up vectors
        m_up = { 0, 1, 0 };
        m_front.x = cos(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));
        m_front.y = sin(glm::radians(m_rotation.y));
        m_front.z = sin(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));
        m_front = glm::normalize(m_front);
    }
    m_previousMousePosition = mousePos;


    // WASD
    bool update = false;
    glm::vec3 offset = { 0, 0, 0 };
    if (window->IsHeld(Input::KEYBOARD::W)) {
        offset += m_front;
        update = true;
    }
    if (window->IsHeld(Input::KEYBOARD::S)) {
        offset -= m_front;
        update = true;
    }
    if (window->IsHeld(Input::KEYBOARD::A)) {
        offset -= glm::normalize(glm::cross(m_front, m_up));
        update = true;
    }
    if (window->IsHeld(Input::KEYBOARD::D)) {
        offset += glm::normalize(glm::cross(m_front, m_up));
        update = true;
    }
    if (window->IsHeld(Input::KEYBOARD::LEFT_CONTROL)) {
        offset -= m_up;
        update = true;
    }
    if (window->IsHeld(Input::KEYBOARD::SPACE)) {
        offset += m_up;
        update = true;
    }

    if (update && offset != glm::vec3{0, 0, 0}) {
        offset = glm::normalize(offset);
        m_position += offset * m_walkSpeed * dt;
    }
}

void Camera::SetPosition(glm::vec3 position) {
    m_position = position;
}
void Camera::SetRotation(glm::vec3 rotation) {
    m_rotation = rotation;
}
void Camera::Update(float aspectRatio) {
    // imgui settings
    if (ImGui::Begin("Settings")) {
        if (ImGui::BeginTabBar("")) {
            if (ImGui::BeginTabItem("Info")) {
                ImGui::Text(std::format("Camera position: (%f, %f, %f)", m_position.x, m_position.y, m_position.z).c_str());
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Camera")) {
                ImGui::SliderFloat("Movement Speed", &m_walkSpeed, 0.0f, 1000.0f);
                ImGui::SliderFloat("Sensitivity", &m_lookSpeed, 0.0f, 1000.0f);
                ImGui::SliderFloat("FOV", &m_parameters.fov, 0.0f, 200.0f);

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    m_projection = glm::perspective(glm::radians(m_parameters.fov), aspectRatio, m_parameters.planeNear, m_parameters.planeFar);
    m_view = glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::SetParameters(CameraParameters params) {
    m_parameters = params;
}