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
	glm::vec3 GetPosition() { return m_cam.position; };
	void SetRotation(glm::vec3 rotation);
	glm::vec3 GetRotation() { return m_cam.rotation; };

	void Update(float aspectRatio);
	void SetParameters(CameraParameters params);

	glm::mat4& GetView() { return currentCam->view; };
	glm::mat4& GetProjection() { return currentCam->projection; };
private:
	CameraParameters m_parameters;

	struct CamVars {
		glm::vec3 position;
		glm::vec3 rotation;

		glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 front;
		glm::vec3 up;
	};
	CamVars m_cam;
	CamVars m_freecam;
	CamVars* currentCam = &m_cam;
	bool m_freecammode = false;

	glm::vec2 m_previousMousePosition;
	float m_previousTime;

	float m_walkSpeed = 8000;
	float m_lookSpeed = 0.15;
};