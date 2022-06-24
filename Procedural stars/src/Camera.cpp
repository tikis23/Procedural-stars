#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/orthonormalize.hpp>
#include "imgui/imgui.h"
#include <format>

Camera::Camera(const glm::dvec3& position, const glm::dmat3& orientation) {
    m_freecam = false;
    m_speed = 10000;
    m_sensitivity = 100;
    m_rollSpeed = 40;
    m_fov = 60;
	SetPosition(position);
	SetOrientation(orientation);
}

Camera::~Camera() {

}

void Camera::Update(Window* window, float aspectRatio) {
	// imgui settings
    if (ImGui::Begin("Settings")) {
        if (ImGui::BeginTabBar("")) {
            if (ImGui::BeginTabItem("Info")) {
                ImGui::Text(std::format("pos: ({}, {}, {})", m_position.x, m_position.y, m_position.z).c_str());
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Camera")) {
                if (m_speed < 2)
                    ImGui::DragScalar("Movement Speed", ImGuiDataType_Double, &m_speed, 5 * 0.001);
                else if (m_speed < 5)
                    ImGui::DragScalar("Movement Speed", ImGuiDataType_Double, &m_speed, 5 * 0.0025);
                else if (m_speed < 20)
                    ImGui::DragScalar("Movement Speed", ImGuiDataType_Double, &m_speed, 5 * 0.01);
                else if (m_speed < 200)
                    ImGui::DragScalar("Movement Speed", ImGuiDataType_Double, &m_speed, 5 * 0.1);
                else if (m_speed < 800)
                    ImGui::DragScalar("Movement Speed", ImGuiDataType_Double, &m_speed, 5 * 1.0);
                else
                    ImGui::DragScalar("Movement Speed", ImGuiDataType_Double, &m_speed, 5 * 2.5);

                ImGui::DragScalar("Sensitivity", ImGuiDataType_Double, &m_sensitivity, 0.1);
                ImGui::DragScalar("Roll speed", ImGuiDataType_Double, &m_rollSpeed, 0.1);
                ImGui::DragFloat("FOV", &m_fov, 0.1f, 30, 200.0f);
                if (ImGui::Checkbox("Freecam", &m_freecam)) {
                    if (m_freecam) {
                        m_restoredOrientation = m_orientation;
                        m_restoredPosition = m_position;
                        m_restoredProj = m_proj;
                        m_restoredView = m_view;
                    }
                    else {
                        m_orientation = m_restoredOrientation;
                        m_position = m_restoredPosition;
                    }
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    // get input
    glm::dvec3 front = GetForwardVector();
    glm::dvec3 up = GetUpVector();
    glm::dvec3 left = GetLeftVector();
    double time = glfwGetTime();
    double dt = time - m_previousTime;
    m_previousTime = time;

    // rotation
    glm::vec2 mousePos = window->GetMousePosition();
    if (window->IsHeld(Input::MOUSE::RIGHT)) {
        // add rotation
        glm::vec2 offset = (m_previousMouse - mousePos) * (float)m_sensitivity * 0.001f;
        RotateEuler({ offset.y, offset.x, 0 });
    }
    if (window->IsHeld(Input::KEYBOARD::Q))
        RotateEuler({ 0, 0, -m_rollSpeed * dt });
    if (window->IsHeld(Input::KEYBOARD::E))
        RotateEuler({ 0, 0,  m_rollSpeed * dt });
    m_previousMouse = mousePos;
    
    // movement
    glm::dvec3 offset = { 0, 0, 0 };
    if (window->IsHeld(Input::KEYBOARD::W))
        offset += front;
    if (window->IsHeld(Input::KEYBOARD::S))
        offset -= front;
    if (window->IsHeld(Input::KEYBOARD::A))
        offset += left;
    if (window->IsHeld(Input::KEYBOARD::D))
        offset -= left;
    if (window->IsHeld(Input::KEYBOARD::LEFT_CONTROL))
        offset -= up;
    if (window->IsHeld(Input::KEYBOARD::SPACE))
        offset += up;
    if (window->IsHeld(Input::KEYBOARD::LEFT_SHIFT))
        offset *= 2;
    if (offset != glm::dvec3{0, 0, 0}) {
        offset = glm::normalize(offset);
        SetPosition(m_position + offset * m_speed * dt);
    }


    // update matrices
	m_view = glm::lookAt(m_position, m_position + front, up);
	m_proj = glm::perspective(glm::radians(m_fov), aspectRatio, .1f, 100000000000000.0f);
}

glm::dmat3 Camera::GetOrientation() const {
	return m_freecam ? m_restoredOrientation : m_orientation;
}

void Camera::SetOrientation(const glm::dmat3& orientation) {
	m_orientation = orientation;
}

glm::dvec3 Camera::GetPosition() const {
	return m_freecam ? m_restoredPosition : m_position;
}

void Camera::SetPosition(const glm::dvec3& position) {
	m_position = position;
}

void Camera::RotateEuler(const glm::dvec3& degrees) {
    glm::dmat3 rotx = glm::rotate(glm::dmat4(1), glm::radians(degrees.x), glm::dvec3(1, 0, 0));
    glm::dmat3 roty = glm::rotate(glm::dmat4(1), glm::radians(degrees.y), glm::dvec3(0, 1, 0));
    glm::dmat3 rotz = glm::rotate(glm::dmat4(1), glm::radians(degrees.z), glm::dvec3(0, 0, 1));
    Rotate(roty * rotx * rotz);
}

void Camera::Rotate(const glm::dmat3& rotation) {
	m_orientation *= rotation;
	m_orientation = glm::orthonormalize(m_orientation);
}

void Camera::Translate(const glm::dvec3& translation) {
	m_position += m_orientation * translation;
}

glm::dvec3 Camera::GetForwardVector() const {
	return glm::column(m_orientation, 2);
}

glm::dvec3 Camera::GetUpVector() const {
	return glm::column(m_orientation, 1);
}

glm::dvec3 Camera::GetLeftVector() const {
	return glm::column(m_orientation, 0);
}

glm::mat4& Camera::GetViewMatrix() {
	return m_view;
}

glm::mat4& Camera::GetProjMatrix() {
	return m_proj;
}

glm::mat4& Camera::GetOriginalViewMatrix() {
    if (m_freecam)
        return m_restoredView;
    else
        return m_view;
}

glm::mat4& Camera::GetOriginalProjMatrix() {
    if (m_freecam)
        return m_restoredProj;
    else
        return m_proj;
}