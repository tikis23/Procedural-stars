#include "TerrainTopology.h"
#include <GL/glew.h>
TerrainTopology::TerrainTopology(unsigned int detailNorth, unsigned int detailEast, unsigned int detailSouth, unsigned int detailWest) {
    TriangleFanList triangleFans;

    // Build the patch body (without edges)
    for (unsigned int x = 2; x < PATCH_VERTICES - 2; x += 2) {
        for (unsigned int y = 2; y < PATCH_VERTICES - 2; y += 2) {
            AddTriangleFan(triangleFans);
            AddTriangleFanVertex(triangleFans, x, y);
            AddTriangleFanVertex(triangleFans, x + 1, y + 1);
            AddTriangleFanVertex(triangleFans, x, y + 1);
            AddTriangleFanVertex(triangleFans, x - 1, y + 1);
            AddTriangleFanVertex(triangleFans, x - 1, y);
            AddTriangleFanVertex(triangleFans, x - 1, y - 1);
            AddTriangleFanVertex(triangleFans, x, y - 1);
            AddTriangleFanVertex(triangleFans, x + 1, y - 1);
            AddTriangleFanVertex(triangleFans, x + 1, y);
            AddTriangleFanVertex(triangleFans, x + 1, y + 1);
        }
    }

    // Build all four edges
    BuildEdge(triangleFans, NORTH, detailNorth);
    BuildEdge(triangleFans, EAST, detailEast);
    BuildEdge(triangleFans, SOUTH, detailSouth);
    BuildEdge(triangleFans, WEST, detailWest);

    // Create an index array
    int n = 0;
    for (unsigned int i = 0; i < triangleFans.size(); i++) {
        for (unsigned int j = 2; j < triangleFans[i].size(); j++) {
            m_indices.push_back(triangleFans[i][0]);
            m_indices.push_back(triangleFans[i][j - 1]);
            m_indices.push_back(triangleFans[i][j]);
        }
    }
    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned short), &m_indices[0], GL_STATIC_DRAW);
}

TerrainTopology::~TerrainTopology() {
    glDeleteBuffers(1, &m_EBO);
}

void TerrainTopology::RotateIndices(unsigned int& x, unsigned int& y, const unsigned int rotation) {
	switch (rotation)
	{
	case NORTH:
	default:
		return;
	case EAST:
		std::swap(x, y);
		y = PATCH_SIZE - y;
		return;
	case SOUTH:
		x = PATCH_SIZE - x;
		y = PATCH_SIZE - y;
		return;
	case WEST:
		std::swap(x, y);
		x = PATCH_SIZE - x;
		return;
	}
}

void TerrainTopology::AddTriangleFan(TriangleFanList& triangleFans) {
    triangleFans.push_back(TriangleFan());
}

void TerrainTopology::AddTriangleFanVertex(TriangleFanList& triangleFans, unsigned int x, unsigned int y, const unsigned int rotation) {
    RotateIndices(x, y, rotation);
    triangleFans.back().push_back(INDEX1D(x, y));
}

void TerrainTopology::BuildEdge(TriangleFanList& triangleFans, const unsigned int side, const unsigned int detail) {
    if (detail == 0 || detail > MAX_DETAIL_DIFFERENCE) {
        for (unsigned int x = 0; x < PATCH_SIZE - 1; x += 2) {
            AddTriangleFan(triangleFans);
            AddTriangleFanVertex(triangleFans, x + 1, PATCH_SIZE - 1, side);
            for (int d = -1; d <= 1; d++)
                AddTriangleFanVertex(triangleFans, x + 1 - d, PATCH_SIZE, side);

            if (x > 0) {
                AddTriangleFan(triangleFans);
                AddTriangleFanVertex(triangleFans, x, PATCH_SIZE, side);
                for (int d = -1; d <= 1; d++)
                    AddTriangleFanVertex(triangleFans, x + d, PATCH_SIZE - 1, side);
            }
        }
        return;
    }

    const int step = 1 << detail;
    const int halfStep = step / 2;

    unsigned int x = 0;
    for (x = 0; x < PATCH_SIZE; x += step) {
        AddTriangleFan(triangleFans);
        AddTriangleFanVertex(triangleFans, x, PATCH_SIZE, side);
        AddTriangleFanVertex(triangleFans, x + halfStep, PATCH_SIZE - 1, side);
        AddTriangleFanVertex(triangleFans, x + step, PATCH_SIZE, side);

        if (x > 0) {
            AddTriangleFan(triangleFans);
            AddTriangleFanVertex(triangleFans, x, PATCH_SIZE, side);
            for (int d = -halfStep; d <= halfStep; d++)
                AddTriangleFanVertex(triangleFans, x + d, PATCH_SIZE - 1, side);
        }
    }

    if (step > 2) {
        AddTriangleFan(triangleFans);
        AddTriangleFanVertex(triangleFans, 0, PATCH_SIZE, side);
        for (int d = 1; d <= halfStep; d++)
            AddTriangleFanVertex(triangleFans, d, PATCH_SIZE - 1, side);

        AddTriangleFan(triangleFans);
        AddTriangleFanVertex(triangleFans, PATCH_SIZE, PATCH_SIZE, side);
        for (int d = -halfStep; d <= -1; d++)
            AddTriangleFanVertex(triangleFans, PATCH_SIZE + d, PATCH_SIZE - 1, side);
    }
}
