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
#define NORTH_WEST                      0
#define NORTH_EAST                      1
#define SOUTH_EAST                      2
#define SOUTH_WEST                      3
#define CENTER                          4
#define MIRROR(s)                       (((s) + 2) % 4)
#define ADJACENT(s, q)                  ((4 + (q) - (s)) % 4 <= 1)
#define REFLECT(s, q)                   ((s) % 2 ? ((q) % 2 ? (q) - 1 : (q) + 1) : 3 - (q))
#define INDEX1D(x, y)                   ((x) + (y) * PATCH_VERTICES)
#define INDEX2D(i)                      ((i) % PATCH_VERTICES), ((i) / PATCH_VERTICES)

typedef std::vector<unsigned short> TriangleFan;
typedef std::vector<TriangleFan>    TriangleFanList;

class TerrainTopology {
public:
	TerrainTopology(unsigned int detailNorth, unsigned int detailEast, unsigned int detailSouth, unsigned int detailWest);
	~TerrainTopology();

	const std::vector<glm::ivec3>& GetVertices()const { return m_vertices; };
	const std::vector<unsigned short>& GetIndices()const { return m_indices; };
private:
	std::vector<glm::ivec3> m_vertices;
	std::vector<unsigned short> m_indices;

	void RotateIndices(unsigned int& x, unsigned int& y, const unsigned int rotation);
	void AddTriangleFan(TriangleFanList& triangleFans);
	void AddTriangleFanVertex(TriangleFanList& triangleFans, unsigned int x, unsigned int y, const unsigned int rotation = 0);
	void BuildEdge(TriangleFanList& triangleFans, const unsigned int side, const unsigned int detail = 0);
};