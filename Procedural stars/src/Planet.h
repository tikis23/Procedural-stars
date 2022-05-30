#pragma once

#include <vector>
#include "QuadTree.h"
#include "glm/glm.hpp"

enum PLANET_FACE {
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
	FRONT,
	BACK
};

class Planet {
public:
	Planet();
	~Planet();
	QuadTree* GetTree() { return m_tree; };
	void SetPosition(glm::vec3 pos) { m_position = pos; };
	void SetRotation(glm::vec3 rot) { m_rotation = rot; };
	glm::vec3 GetPosition() { return m_position; };
	glm::vec3 GetRotation() { return m_rotation; };
	glm::mat4& GetPositionModelMatrix() { return m_modelPos; };
	glm::mat4& GetRotationModelMatrix() { return m_modelRot; };
	void Update();
	void Render(glm::vec3 cameraPos);
private:
	QuadTree* m_tree;
	void GenerateMesh(QUADTREE_NODE* node);
	void RenderLod(QUADTREE_NODE* node, glm::vec3 cameraPos);
	glm::vec3 ApplyNoise(glm::vec3 pos);
	void MeshCreateData(Mesh* mesh, int face, glm::vec3 localPosition, glm::vec3 dimensions);

	float m_radius;
	unsigned int m_lodAmount;
	unsigned int m_nodeVertexAmount;
	glm::mat4 m_modelPos;
	glm::mat4 m_modelRot;
	glm::vec3 m_position;
	glm::vec3 m_rotation;
};