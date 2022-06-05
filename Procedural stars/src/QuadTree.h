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
	Mesh* mesh = nullptr;
	QUADTREE_NODE* parent = nullptr;
	QUADTREE_NODE* child[4] = { nullptr };
	void Split() {
		if (type == QUADTREENODETYPE_LEAF) {
			type = QUADTREENODETYPE_NODE;
			for (int i = 0; i < 4; i++) {
				child[i] = new QUADTREE_NODE;
				child[i]->type = QUADTREENODETYPE_LEAF;
				child[i]->level = level + 1;
				child[i]->face = face;
				child[i]->parent = this;
				child[i]->size = size / 2;
				child[i]->localPosition = { 
					localPosition.x + child[i]->size * ((i % 2) * 2 - 1),
					localPosition.y + child[i]->size * ((i / 2) * 2 - 1)
				};
			}
		}
	};
	void Delete() {

	}
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