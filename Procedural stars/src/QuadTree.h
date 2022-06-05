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
	glm::vec3 localPosition;
	glm::vec3 dimensions;
	glm::vec3 minPoint;
	glm::vec3 maxPoint;
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
				child[i]->dimensions = dimensions * 0.5f;
				child[i]->localPosition = localPosition;
				switch (face)
				{
				case 0:
				case 1:
					child[i]->localPosition.x += child[i]->dimensions.x * ((i % 2) * 2 - 1);
					child[i]->localPosition.z += child[i]->dimensions.z * ((i / 2) * 2 - 1);
					break;
				case 2:
				case 3:
					child[i]->localPosition.y += child[i]->dimensions.y * ((i % 2) * 2 - 1);
					child[i]->localPosition.z += child[i]->dimensions.z * ((i / 2) * 2 - 1);
					break;
				case 4:
				case 5:
					child[i]->localPosition.y += child[i]->dimensions.y * ((i % 2) * 2 - 1);
					child[i]->localPosition.x += child[i]->dimensions.x * ((i / 2) * 2 - 1);
					break;
				default:
					break;
				}
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