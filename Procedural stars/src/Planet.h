#pragma once

#include <vector>
#include "Shader.h"
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
	void Render(glm::vec3 cameraPos, Shader* shader);
private:
	QuadTree* m_tree;
	void GenerateMesh(QUADTREE_NODE* node);
	void GetLod(QUADTREE_NODE* node, std::vector<QUADTREE_NODE*>& queue, glm::vec3 cameraPos);
	void MeshCreateData(QUADTREE_NODE* node,void* ptr);
	void CreateFace(std::vector<Vertex>* data, int face);
	bool Cleanup(QUADTREE_NODE* node, double currentTime, int& meshCount);
	QUADTREE_NODE* GetNeighbor(QUADTREE_NODE* node, int dir);

	float m_radius;
	float m_maxHeight;
	double m_nodeCleanupTimer;

	int m_lodAmount;
	int m_nodeVertexAmount;
	glm::mat4 m_modelPos;
	glm::mat4 m_modelRot;
	glm::vec3 m_position;
	glm::vec3 m_rotation;
};