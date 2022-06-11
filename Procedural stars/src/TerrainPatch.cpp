#include "QuadTree.h"
#include "TerrainPatch.h"
#include "glm/gtx/norm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <FastNoise/FastNoise.h>
#include "Timer.h"

TerrainPatch::TerrainPatch(QuadTreeNode* node) {
	ScopedTimer timer("TerrainPatchGeneration");
	m_node = node;
	m_patchRotation = node->m_patchRotation;

	auto generator = FastNoise::New<FastNoise::Perlin>();
	// big hills
	auto bdomainScale = FastNoise::New<FastNoise::DomainScale>();
	bdomainScale->SetSource(generator);
	bdomainScale->SetScale(10);
	auto bfractalFbm = FastNoise::New<FastNoise::FractalFBm>();
	bfractalFbm->SetSource(bdomainScale);
	bfractalFbm->SetGain(0.4);
	bfractalFbm->SetWeightedStrength(0);
	bfractalFbm->SetOctaveCount(4);
	bfractalFbm->SetLacunarity(2.5);
	auto bmodifierRemap = FastNoise::New<FastNoise::Remap>();
	bmodifierRemap->SetSource(bfractalFbm);
	bmodifierRemap->SetRemap(-1, 1, -2, 1);
	auto bblendMaxSmooth = FastNoise::New<FastNoise::MaxSmooth>();
	bblendMaxSmooth->SetLHS(bmodifierRemap);
	bblendMaxSmooth->SetRHS(-0.2);
	bblendMaxSmooth->SetSmoothness(0.1);

	// med hills
	auto mdomainScale = FastNoise::New<FastNoise::DomainScale>();
	mdomainScale->SetSource(generator);
	mdomainScale->SetScale(20);
	auto mfractalFbm = FastNoise::New<FastNoise::FractalFBm>();
	mfractalFbm->SetSource(mdomainScale);
	mfractalFbm->SetGain(0.5);
	mfractalFbm->SetWeightedStrength(0);
	mfractalFbm->SetOctaveCount(4);
	mfractalFbm->SetLacunarity(2);
	auto mmodifierRemap = FastNoise::New<FastNoise::Remap>();
	mmodifierRemap->SetSource(mfractalFbm);
	mmodifierRemap->SetRemap(-1, 1, -0.2, 0.1);

	// low hills
	auto ldomainScale = FastNoise::New<FastNoise::DomainScale>();
	ldomainScale->SetSource(generator);
	ldomainScale->SetScale(40);
	auto lfractalFbm = FastNoise::New<FastNoise::FractalFBm>();
	lfractalFbm->SetSource(ldomainScale);
	lfractalFbm->SetGain(0.5);
	lfractalFbm->SetWeightedStrength(0);
	lfractalFbm->SetOctaveCount(5);
	lfractalFbm->SetLacunarity(3);
	auto lmodifierRemap = FastNoise::New<FastNoise::Remap>();
	lmodifierRemap->SetSource(lfractalFbm);
	lmodifierRemap->SetRemap(-1, 1, -0.03, 0.03);

	// extra noise
	auto edomainScale = FastNoise::New<FastNoise::DomainScale>();
	edomainScale->SetSource(generator);
	edomainScale->SetScale(50);
	auto efractalFbm = FastNoise::New<FastNoise::FractalFBm>();
	efractalFbm->SetSource(edomainScale);
	efractalFbm->SetGain(2);
	efractalFbm->SetWeightedStrength(0);
	efractalFbm->SetOctaveCount(5);
	efractalFbm->SetLacunarity(3);
	auto emodifierRemap = FastNoise::New<FastNoise::Remap>();
	emodifierRemap->SetSource(efractalFbm);
	emodifierRemap->SetRemap(-1, 1, -0.002, 0.002);

	// combine
	auto cAdd1 = FastNoise::New<FastNoise::Add>();
	cAdd1->SetLHS(bblendMaxSmooth);
	cAdd1->SetRHS(mmodifierRemap);
	auto cAdd2 = FastNoise::New<FastNoise::Add>();
	cAdd2->SetLHS(cAdd1);
	cAdd2->SetRHS(lmodifierRemap);
	auto cAdd3 = FastNoise::New<FastNoise::Add>();
	cAdd3->SetLHS(cAdd2);
	cAdd3->SetRHS(emodifierRemap);

	for (int x = 0; x < PATCH_VERTICES; x++) {
		for (int y = 0; y < PATCH_VERTICES; y++) {
			glm::vec3 vertPos = (glm::vec3{
				(double)(x - PATCH_SIZE_HALF) * node->m_size / PATCH_SIZE_HALF + node->m_position.x,
				1,
				(double)(y - PATCH_SIZE_HALF) * node->m_size / PATCH_SIZE_HALF + node->m_position.y
			});
			TVertex vert = {vertPos * 8000.f, {0, 0, 0} };
			vertPos = m_patchRotation * glm::vec4(vertPos, 1);
			vert.position += 400.f * glm::normalize(vert.position) * cAdd3->GenSingle3D(vertPos.x, vertPos.y, vertPos.z, 1);
			m_vertexData.push_back(vert);
		}
	}

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(TVertex), &m_vertexData[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(TVertex), (void*)0);
	glEnableVertexAttribArray(0);
}

TerrainPatch::~TerrainPatch() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
}

void TerrainPatch::Render(Shader* shader) {
	TerrainTopology* topology = GetTopology(
		m_node->m_neighborDetailDifferences[0],
		m_node->m_neighborDetailDifferences[1],
		m_node->m_neighborDetailDifferences[2],
		m_node->m_neighborDetailDifferences[3]);
	//TerrainTopology* topology = GetTopology(3, 0, 0, 0);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, topology->GetEBO());
	shader->uniformMatrix4("model", glm::value_ptr(m_patchRotation));
	glm::vec3 color = { 0, 0, 0 };
	if (m_node->m_face == FACE_TOP) // red
		color = { 1, 0, 0 };
	if (m_node->m_face == FACE_BOTTOM) // green
		color = { 0, 1, 0 };
	if (m_node->m_face == FACE_FRONT) // blue
		color = { 0, 0, 1 };
	if (m_node->m_face == FACE_BACK) // yellow
		color = { 1, 1, 0 };
	if (m_node->m_face == FACE_LEFT) // purple
		color = { 1, 0, 1 };
	if (m_node->m_face == FACE_RIGHT) // cyan
		color = { 0, 1, 1 };
	shader->uniform3f("color", color);
	glDrawElements(GL_TRIANGLES, topology->GetIndicesAmount(), GL_UNSIGNED_SHORT, (void*)0);
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

	position = glm::dmat4(m_patchRotation) * glm::dvec4(center, 1);
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