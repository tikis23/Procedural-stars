#pragma once
#include <vector>
#include "glm/glm.hpp"

#define MAX_DETAIL_DIFFERENCE           4
#define PATCH_SIZE                      (1 << MAX_DETAIL_DIFFERENCE) // 2^MAX_DETAIL_DIFFERENCE
#define PATCH_SIZE_HALF                 (PATCH_SIZE / 2)
#define PATCH_VERTICES                  (PATCH_SIZE + 1)
#define PATCH_VERTICES_TOTAL            (PATCH_VERTICES * PATCH_VERTICES)
#define NORTH                           0
#define EAST                            1
#define SOUTH                           2
#define WEST                            3

#define INDEX1D(x, y)                   ((x) + (y) * PATCH_VERTICES)
#define INDEX2D(i)                      ((i) % PATCH_VERTICES), ((i) / PATCH_VERTICES)

#define FACE_TOP    0
#define FACE_BOTTOM 1
#define FACE_LEFT   2
#define FACE_RIGHT  3
#define FACE_FRONT  4
#define FACE_BACK   5

typedef std::vector<unsigned short> TriangleFan;
typedef std::vector<TriangleFan>    TriangleFanList;

class TerrainTopology {
public:
	TerrainTopology(unsigned int detailNorth, unsigned int detailEast, unsigned int detailSouth, unsigned int detailWest);
	~TerrainTopology();

	//const std::vector<glm::ivec3>& GetVertices()const { return m_vertices; };
	//const std::vector<unsigned short>& GetIndices()const { return m_indices; };
	const unsigned int& GetEBO()const { return m_EBO; };
	unsigned int GetIndicesAmount()const { return m_indices.size(); };
private:
	std::vector<glm::ivec3> m_vertices;
	std::vector<unsigned short> m_indices;
	unsigned int m_EBO;
	void RotateIndices(unsigned int& x, unsigned int& y, const unsigned int rotation);
	void AddTriangleFan(TriangleFanList& triangleFans);
	void AddTriangleFanVertex(TriangleFanList& triangleFans, unsigned int x, unsigned int y, const unsigned int rotation = 0);
	void BuildEdge(TriangleFanList& triangleFans, const unsigned int side, const unsigned int detail = 0);
};