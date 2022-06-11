#pragma once

#include "QuadTree.h"
#include "glm/glm.hpp"
#include "Shader.h"


class TerrainFace {
public:
	TerrainFace(int face);
	~TerrainFace();
	void Update(glm::dvec3 cameraPos);
	void Render(Shader* shader);
	QuadTreeNode* m_rootNode;
	void Connect(const unsigned int side, TerrainFace* face);
private:
	int m_face;

	int m_splitsRemaining;
};