#pragma once

#include "QuadTree.h"
#include "glm/glm.hpp"
#include "Shader.h"

#define FACE_TOP    0
#define FACE_BOTTOM 1
#define FACE_LEFT   2
#define FACE_RIGHT  3
#define FACE_FRONT  4
#define FACE_BACK   5

class TerrainFace {
public:
	TerrainFace(int face);
	~TerrainFace();
	void Update();
	void Render(Shader* shader);
private:
	int m_face;
	QuadTreeNode* m_rootNode;

	int m_splitsRemaining;
};