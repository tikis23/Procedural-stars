#pragma once

#include <vector>
#include "Mesh.h"
#include "glm/glm.hpp"

class Planet {
public:
	Planet();
	~Planet();
	std::vector<Mesh*> GetMeshes() { return m_meshes; };
	float GetRadius() { return m_radius; };
	float GetRadiusMax() { return m_radiusMax; };
	glm::vec3 GetPosition() { return m_position; };
	void SetPosition(glm::vec3 pos) { m_position = pos; };
	glm::vec3 GetRotation() { return m_rotation; };
	void SetRotation(glm::vec3 rot) { m_rotation = rot; };
	glm::mat4& GetPositionModelMatrix() { return m_modelPos; };
	glm::mat4& GetRotationModelMatrix() { return m_modelRot; };
	void Update();
private:
	static std::vector<Mesh*> m_meshes;

	glm::mat4 m_modelPos;
	glm::mat4 m_modelRot;
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	float m_radius;
	float m_radiusMax;
};