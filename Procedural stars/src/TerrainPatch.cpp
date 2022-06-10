#include "QuadTree.h"
#include "TerrainPatch.h"
#include "glm/gtx/norm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
TerrainPatch::TerrainPatch(QuadTreeNode* node) {
	TerrainTopology* topology = GetTopology(0, 0, 0, 0);
	const std::vector<glm::ivec3>& topologyVertices = topology->GetVertices();
	m_patchRotation = node->m_patchRotation;

	for (int x = 0; x < PATCH_VERTICES; x++) {
		for (int y = 0; y < PATCH_VERTICES; y++) {
			m_vertexData.push_back({ 8000.f * (glm::vec3{ (float)x / PATCH_VERTICES, 1, (float)y / PATCH_VERTICES }), {0, 0, 0}});
		}
	}

	m_indices = topology->GetIndices();

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(TVertex), &m_vertexData[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(TVertex), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned short), &m_indices[0], GL_STATIC_DRAW);
}

TerrainPatch::~TerrainPatch() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
}

void TerrainPatch::Render(Shader* shader) {
	glBindVertexArray(m_VAO);
	shader->uniformMatrix4("model", glm::value_ptr(m_patchRotation));
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_SHORT, (void*)0);
}

void TerrainPatch::GetBoundingSphere(glm::dvec3& position, double& radius) {
	glm::dvec3 center(0, 0, 0);
	double maxradius = 0;
	// get center
	for (int i = 0; i < m_vertexData.size(); i++)
		center += m_vertexData[i].position;
	center /= (double)m_vertexData.size();
	
	// get radius
	for (int i = 0; i < m_vertexData.size(); i++)
		maxradius = std::max(maxradius, glm::length2((glm::dvec3)m_vertexData[i].position - center));
	maxradius = sqrt(maxradius);

	position = center;
	radius = maxradius;
}

inline glm::vec3 TerrainPatch::MapToSphere(const glm::vec3& pos) {
	double sqrx = pos.x * pos.x;
	double sqry = pos.y * pos.y;
	double sqrz = pos.z * pos.z;
	return {
		pos.x * sqrt(1 - sqry / 2 - sqrz / 2 + sqry * sqrz / 3),
		pos.y * sqrt(1 - sqrz / 2 - sqrx / 2 + sqrz * sqrx / 3),
		pos.z * sqrt(1 - sqrx / 2 - sqry / 2 + sqrx * sqry / 3),
	};
}

TerrainTopology* TerrainPatch::GetTopology(unsigned int detailNorth, unsigned int detailEast, unsigned int detailSouth, unsigned int detailWest) {
	TerrainTopology* topology = sm_topology[detailNorth][detailEast][detailSouth][detailWest];
	if (topology == nullptr) {
		topology = new TerrainTopology(detailNorth, detailEast, detailSouth, detailWest);
		sm_topology[detailNorth][detailEast][detailSouth][detailWest] = topology;
	}
	return topology;
}

TerrainTopology* TerrainPatch::sm_topology[MAX_DETAIL_DIFFERENCE + 1][MAX_DETAIL_DIFFERENCE + 1][MAX_DETAIL_DIFFERENCE + 1][MAX_DETAIL_DIFFERENCE + 1] = { nullptr };