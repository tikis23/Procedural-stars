#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "TerrainTopology.h"
#include "Shader.h"

#define MAP_SIZE 4

class QuadTreeNode;

struct TVertex {
	glm::vec3 position;
	glm::vec2 texcoord;
};

class TerrainPatch {
public:
	TerrainPatch(QuadTreeNode* node);
	~TerrainPatch();

	void Render(Shader* shader);
	void GetBoundingSphere(glm::dvec3& position, double& radius);
	inline glm::vec3 MapToSphere(const glm::vec3& pos);
private:
	std::vector<TVertex> m_vertexData;
	glm::mat4 m_patchRotation;
	QuadTreeNode* m_node;
	unsigned int m_VBO;
	unsigned int m_VAO;
	unsigned int m_mapTexture;

	static bool sm_topologiesCreated;
	static TerrainTopology* sm_topology[MAX_DETAIL_DIFFERENCE][MAX_DETAIL_DIFFERENCE][MAX_DETAIL_DIFFERENCE][MAX_DETAIL_DIFFERENCE];
	static TerrainTopology* GetTopology(unsigned int detailNorth, unsigned int detailEast, unsigned int detailSouth, unsigned int detailWest);
};