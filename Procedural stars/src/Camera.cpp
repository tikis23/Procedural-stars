#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include "imgui/imgui.h"
#include <format>
Camera::Camera(CameraParameters params, glm::vec3 position, glm::vec3 rotation) {
    SetParameters(params);
    m_cam.position = position;
    m_cam.rotation = rotation;
    if (m_cam.rotation.y > 89.0f)
        m_cam.rotation.y = 89.0f;
    if (m_cam.rotation.y < -89.0f)
        m_cam.rotation.y = -89.0f;
    m_cam.rotation.x = fmodf(m_cam.rotation.x, 360.0f);
    m_cam.rotation.z = fmodf(m_cam.rotation.z, 360.0f);
    // calculate front && up vectors
    m_cam.up = { 0, 1, 0 };
    m_cam.front.x = cosf(glm::radians(m_cam.rotation.x)) * cosf(glm::radians(m_cam.rotation.y));
    m_cam.front.y = sinf(glm::radians(m_cam.rotation.y));
    m_cam.front.z = sinf(glm::radians(m_cam.rotation.x)) * cosf(glm::radians(m_cam.rotation.y));
    m_cam.front = glm::normalize(m_cam.front);
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
        // add rotation
        glm::vec2 offset = mousePos - m_previousMousePosition;
        currentCam->rotation.x += offset.x * m_lookSpeed;
        currentCam->rotation.y += offset.y * m_lookSpeed;
        currentCam->rotation.x = fmodf(currentCam->rotation.x, 360.0f);
        if (currentCam->rotation.y > 89.0f)
            currentCam->rotation.y = 89.0f;
        if (currentCam->rotation.y < -89.0f)
            currentCam->rotation.y = -89.0f;


        // apply rotation
        glm::vec3 temp_rotation = currentCam->rotation;
        currentCam->front.x = cos(glm::radians(temp_rotation.x)) * cos(glm::radians(temp_rotation.y));
        currentCam->front.y = sin(glm::radians(temp_rotation.y));
        currentCam->front.z = sin(glm::radians(temp_rotation.x)) * cos(glm::radians(temp_rotation.y));
        currentCam->front = glm::normalize(currentCam->front);
    }
    m_previousMousePosition = mousePos;
    
    // WASD
    bool update = false;
    glm::vec3 offset = { 0, 0, 0 };
    if (window->IsHeld(Input::KEYBOARD::W)) {
        offset += currentCam->front;
        update = true;
    }
    if (window->IsHeld(Input::KEYBOARD::S)) {
        offset -= currentCam->front;
        update = true;
    }
    if (window->IsHeld(Input::KEYBOARD::A)) {
        offset -= glm::normalize(glm::cross(currentCam->front, currentCam->up));
        update = true;
    }
    if (window->IsHeld(Input::KEYBOARD::D)) {
        offset += glm::normalize(glm::cross(currentCam->front, currentCam->up));
        update = true;
    }
    if (window->IsHeld(Input::KEYBOARD::LEFT_CONTROL)) {
        offset -= currentCam->up;
        update = true;
    }
    if (window->IsHeld(Input::KEYBOARD::SPACE)) {
        offset += currentCam->up;
        update = true;
    }

    if (update && offset != glm::vec3{0, 0, 0}) {
        offset = glm::normalize(offset);
        currentCam->position += offset * m_walkSpeed * dt;
    }
}

void Camera::SetPosition(glm::vec3 position) {
    m_cam.position = position;
}
void Camera::SetRotation(glm::vec3 rotation) {
    m_cam.rotation = rotation;
}
void Camera::Update(float aspectRatio) {
    // imgui settings
    if (ImGui::Begin("Settings")) {
        if (ImGui::BeginTabBar("")) {
            if (ImGui::BeginTabItem("Info")) {
                ImGui::Text(std::format("Camera position: ({}, {}, {})", m_cam.position.x, m_cam.position.y, m_cam.position.z).c_str());
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Camera")) {
                if (m_walkSpeed < 2)
                    ImGui::DragFloat("Movement Speed", &m_walkSpeed, 5 * 0.001f, 0, 100000000.f);
                else if (m_walkSpeed < 5)
                    ImGui::DragFloat("Movement Speed", &m_walkSpeed, 5 * 0.0025f, 0, 100000000.f);
                else if (m_walkSpeed < 20)
                    ImGui::DragFloat("Movement Speed", &m_walkSpeed, 5 * 0.01f, 0, 100000000.f);
                else if (m_walkSpeed < 200)
                    ImGui::DragFloat("Movement Speed", &m_walkSpeed, 5 * 0.1f, 0, 100000000.f);
                else if (m_walkSpeed < 800)
                    ImGui::DragFloat("Movement Speed", &m_walkSpeed, 5 * 1.f, 0, 100000000.f);
                else
                    ImGui::DragFloat("Movement Speed", &m_walkSpeed, 5 * 2.5f, 0, 100000000.f);

                ImGui::DragFloat("Sensitivity", &m_lookSpeed, 0.01f, 0, 1000.0f);
                ImGui::DragFloat("FOV", &m_parameters.fov, 0.1f, 30, 200.0f);
                if (ImGui::Checkbox("Freecam", &m_freecammode)) {
                    if (m_freecammode) {
                        m_freecam = m_cam;
                        currentCam = &m_freecam;
                    }
                    else {
                        currentCam = &m_cam;
                    }
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    currentCam->projection = glm::perspective(glm::radians(m_parameters.fov), aspectRatio, m_parameters.planeNear, m_parameters.planeFar);
    currentCam->view = glm::lookAt(currentCam->position, currentCam->position + currentCam->front, currentCam->up);
}

void Camera::SetParameters(CameraParameters params) {
    m_parameters = params;
}