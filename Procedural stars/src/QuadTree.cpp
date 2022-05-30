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