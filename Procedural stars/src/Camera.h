#pragma once
#include "glm/glm.hpp"
#include "Window.h"

class Camera
{
public:
    Camera(const glm::dvec3& position = glm::dvec3(0.0, 0.0, 0.0), const glm::dmat3& orientation = glm::dmat3(1));
    ~Camera();

    void        Update(Window* window, float aspectRatio);
    glm::dmat3  GetOrientation() const;
    void        SetOrientation(const glm::dmat3& orientation);
    glm::dvec3  GetPosition() const;
    void        SetPosition(const glm::dvec3& position);
    void        RotateEuler(const glm::dvec3& degrees);
    void        Rotate(const glm::dmat3& orientation);
    void        Translate(const glm::dvec3& translation);
    glm::dvec3  GetForwardVector() const;
    glm::dvec3  GetUpVector() const;
    glm::dvec3  GetLeftVector() const;
    glm::mat4&  GetViewMatrix();
    glm::mat4&  GetProjMatrix();
    glm::mat4&  GetOriginalViewMatrix();
    glm::mat4&  GetOriginalProjMatrix();
private:
    bool m_freecam;
    double m_speed;
    double m_sensitivity;
    double m_rollSpeed;
    float m_fov;
    double m_previousTime;
    glm::vec2 m_previousMouse;
    glm::mat4 m_view;
    glm::mat4 m_proj;
    glm::dmat3 m_orientation;
    glm::dvec3 m_position;

    glm::dmat3 m_restoredOrientation;
    glm::dvec3 m_restoredPosition;
    glm::mat4 m_restoredView;
    glm::mat4 m_restoredProj;
};