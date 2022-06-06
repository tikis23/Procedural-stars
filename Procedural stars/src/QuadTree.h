#pragma once

#include <vector>
#include "glm/glm.hpp"
#include "Mesh.h"

enum QUADTREE_NODETYPE {
	QUADTREENODETYPE_NODE,
	QUADTREENODETYPE_LEAF
};
struct QUADTREE_NODE {
	int type;
	int level;
	int face;
	glm::vec2 localPosition;
	int size;
	glm::vec3 minPoint;
	glm::vec3 maxPoint;
	bool ignoreRender = false;
	double lastRenderTime = 0;
	Mesh* mesh = nullptr;
	QUADTREE_NODE* parent = nullptr;
	QUADTREE_NODE* child[4] = { nullptr };
	~QUADTREE_NODE();
	void Split();
	void Merge();
};

class QuadTree {
public:
	QuadTree();
	QuadTree(int num_branches);
	~QuadTree();

	QUADTREE_NODE* GetBranch(int index);
	size_t BranchAmount() { return m_branches.size(); };
private:
	std::vector<QUADTREE_NODE*> m_branches;
};