#include "QuadTree.h"
#include <iostream>

QuadTree::QuadTree() {

}
QuadTree::QuadTree(int num_branches) {
	for (int i = 0; i < num_branches; i++) {
		m_branches.push_back(new QUADTREE_NODE);
		m_branches.back()->level = 0;
		m_branches.back()->type = QUADTREENODETYPE_LEAF;
	}
}
QuadTree::~QuadTree() {

}
QUADTREE_NODE* QuadTree::GetBranch(int index) {
	if (index >= 0 && index < m_branches.size())
		return m_branches[index];
	return nullptr;
}

QUADTREE_NODE::~QUADTREE_NODE() {
	if (type == QUADTREENODETYPE_NODE)
		for (int i = 0; i < 4; i++)
			delete child[i];
	if (mesh != nullptr)
		delete mesh;
}
void QUADTREE_NODE::Split() {
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
void QUADTREE_NODE::Merge() {
	type = QUADTREENODETYPE_LEAF;
	for (int i = 0; i < 4; i++) {
		delete child[i];
		child[i] = nullptr;
	}
}