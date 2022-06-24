#pragma once

#include "glm/glm.hpp"
#include "TerrainPatch.h"
#include "Shader.h"

#define MAX_DEPTH 14
#define MAX_SPLITS 4

class QuadTreeNode {
public:
	QuadTreeNode();
	~QuadTreeNode();
	void Update(glm::dvec3 cameraPos);
	void Render(Shader* shader);
	void SetNeighbor(unsigned int side, QuadTreeNode* neighbor);

	QuadTreeNode* m_parent = nullptr;
	QuadTreeNode* m_child[4] = { nullptr };
	QuadTreeNode* m_neighbor[4] = { nullptr };
	int m_neighborDetailDifferences[4];
	TerrainPatch* m_patch = nullptr;

	glm::dvec2 m_position;
	double m_size;
	glm::mat4 m_patchRotation;

	bool m_visible;
	unsigned int m_depth;
	unsigned int m_face;
	unsigned int m_quadrant;
	glm::dvec3 m_boundingSphereCenter;
	double m_boundingSphereRadius;
	static double m_splitDistance;
	static int sm_splitsLeft;

private:

	bool Split();
	void Merge();
	bool IsLeaf()const;
	QuadTreeNode* FindEqualOrHigherNeighbor(unsigned int side);
	void FindNeighbor(const unsigned int side);
	void UpdateNeighborDetail(unsigned int side);
	unsigned int MirrorSide(const unsigned int& side);
	unsigned int MirrorQuadrant(const unsigned int& side, const unsigned int& quadrant);
};