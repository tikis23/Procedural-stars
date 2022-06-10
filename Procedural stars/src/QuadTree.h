#pragma once

#include "glm/glm.hpp"
#include "TerrainPatch.h"

class QuadTreeNode {
public:
	QuadTreeNode();
	~QuadTreeNode();
	void Split();
	void Merge();
	void Update();

	QuadTreeNode* m_parent = nullptr;
	QuadTreeNode* m_child[4] = { nullptr };
	QuadTreeNode* m_neighbour[4] = { nullptr };
	TerrainPatch* m_patch = nullptr;

	glm::dvec2 m_position;
	double m_size;
	glm::mat4 m_patchRotation;

	bool m_visible;
	unsigned int m_depth;
	unsigned int m_quadrant;
	glm::dvec3 m_boundingSphereCenter;
	double m_boundingSphereRadius;
};