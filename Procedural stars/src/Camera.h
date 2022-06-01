#pragma once
#include "glm/glm.hpp"
#include "Window.h"
class Camera {
public:

	struct CameraParameters {
		float fov = 60.0f;
		float planeNear = 0.1f;
		float planeFar = 10000000.0f;
	};

	Camera(CameraParameters params, glm::vec3 position, glm::vec3 rotation);
	~Camera();

	void UpdateInput(Window* window);

	void SetPosition(glm::vec3 position);
	glm::vec3 GetPosition() { return m_position; };
	void SetRotation(glm::vec3 rotation);
	glm::vec3 GetRotation() { return m_rotation; };

	void Update(float aspectRatio);
	void SetParameters(CameraParameters params);

	glm::mat4& GetView() { return m_view; };
	glm::mat4& GetProjection() { return m_projection; };
private:
	CameraParameters m_parameters;

	glm::vec3 m_position;
	glm::vec3 m_rotation;

	glm::mat4 m_view;
	glm::mat4 m_projection;
	glm::vec3 m_front;
	glm::vec3 m_up;

	glm::vec2 m_previousMousePosition;
	float m_previousTime;

	float m_walkSpeed = 300;
	float m_lookSpeed = 0.15;
};