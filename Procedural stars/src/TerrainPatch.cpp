#include "QuadTree.h"
#include "TerrainPatch.h"
#include "glm/gtx/norm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <FastNoise/FastNoise.h>
#include "Timer.h"
#include "TerrainNoise.h"
#include "Mesh.h"
TerrainPatch::TerrainPatch(QuadTreeNode* node) {
	ScopedTimer timer("TerrainPatchGeneration");

	if (!sm_topologiesCreated) {
		for (int i = 0; i < MAX_DETAIL_DIFFERENCE; i++)
			for (int j = 0; j < MAX_DETAIL_DIFFERENCE; j++)
				for (int k = 0; k < MAX_DETAIL_DIFFERENCE; k++)
					for (int l = 0; l < MAX_DETAIL_DIFFERENCE; l++)
						GetTopology(i, j, k, l);
		sm_topologiesCreated = true;
	}

	m_node = node;
	m_patchRotation = node->m_patchRotation;

	// noise
	auto terrainNoise = FastNoise::New<FastNoise::Add>();
	{
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

		// extra extra noise
		auto eedomainScale = FastNoise::New<FastNoise::DomainScale>();
		eedomainScale->SetSource(generator);
		eedomainScale->SetScale(800);
		auto eefractalFbm = FastNoise::New<FastNoise::FractalFBm>();
		eefractalFbm->SetSource(eedomainScale);
		eefractalFbm->SetGain(2);
		eefractalFbm->SetWeightedStrength(0);
		eefractalFbm->SetOctaveCount(5);
		eefractalFbm->SetLacunarity(4);
		auto eemodifierRemap = FastNoise::New<FastNoise::Remap>();
		eemodifierRemap->SetSource(eefractalFbm);
		eemodifierRemap->SetRemap(-1, 1, -0.0001, 0.0001);

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
		terrainNoise->SetLHS(cAdd3);
		terrainNoise->SetRHS(eemodifierRemap);
	}
	// terrain gen
	{
		int dataSize = PATCH_VERTICES * PATCH_VERTICES;
		std::vector<float> noiseDataX(dataSize);
		std::vector<float> noiseDataY(dataSize);
		std::vector<float> noiseDataZ(dataSize);
		for (int x = 0; x < PATCH_VERTICES; x++) {
			for (int y = 0; y < PATCH_VERTICES; y++) {
				int index = x * PATCH_VERTICES + y;
				glm::vec3 vertPos = MapToSphere(glm::vec3{
					(double)(x - PATCH_SIZE_HALF) * node->m_size / PATCH_SIZE_HALF + node->m_position.x,
					1,
					(double)(y - PATCH_SIZE_HALF) * node->m_size / PATCH_SIZE_HALF + node->m_position.y
					});
				vertPos = m_patchRotation * glm::vec4(vertPos, 1);
				noiseDataX[index] = vertPos.x;
				noiseDataY[index] = vertPos.y;
				noiseDataZ[index] = vertPos.z;
			}
		}
		std::vector<float> noiseData(dataSize);
		terrainNoise->GenPositionArray3D(noiseData.data(), dataSize, noiseDataX.data(), noiseDataY.data(), noiseDataZ.data(), 0, 0, 0, 1);
		for (int x = 0; x < PATCH_VERTICES; x++) {
			for (int y = 0; y < PATCH_VERTICES; y++) {
				int index = x * PATCH_VERTICES + y;
				glm::vec3 vertPos(noiseDataX[index], noiseDataY[index], noiseDataZ[index]);
				TVertex vert = { vertPos * 8000.f, {
					(float)y / PATCH_VERTICES,
					(float)x / PATCH_VERTICES
				} };
				vert.position += 400.f * vertPos * noiseData[index];
				m_vertexData.push_back(vert);
			}
		}
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(TVertex), &m_vertexData[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(TVertex), (void*)offsetof(TVertex, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(TVertex), (void*)offsetof(TVertex, texcoord));
		glEnableVertexAttribArray(1);
	}

	// noisemap gen
	{

		int dataSize = PATCH_SIZE * PATCH_SIZE * MAP_SIZE * MAP_SIZE;
		std::vector<float> noiseDataX;
		std::vector<float> noiseDataY;
		std::vector<float> noiseDataZ;
		noiseDataX.reserve(dataSize);
		noiseDataY.reserve(dataSize);
		noiseDataZ.reserve(dataSize);
		for (double x = 0; x < PATCH_SIZE; x += 1.0 / MAP_SIZE) {
			for (double y = 0; y < PATCH_SIZE; y += 1.0 / MAP_SIZE) {
				glm::vec3 vertPos = MapToSphere(glm::vec3{
					(double)(x - PATCH_SIZE_HALF) * node->m_size / PATCH_SIZE_HALF + node->m_position.x,
					1,
					(double)(y - PATCH_SIZE_HALF) * node->m_size / PATCH_SIZE_HALF + node->m_position.y
					});
				vertPos = m_patchRotation * glm::vec4(vertPos, 1);
				noiseDataX.push_back(vertPos.x);
				noiseDataY.push_back(vertPos.y);
				noiseDataZ.push_back(vertPos.z);
			}
		}
		std::vector<float> noiseData(dataSize);
		terrainNoise->GenPositionArray3D(noiseData.data(), dataSize, noiseDataX.data(), noiseDataY.data(), noiseDataZ.data(), 0, 0, 0, 1);
		glGenTextures(1, &m_mapTexture);
		glBindTexture(GL_TEXTURE_2D, m_mapTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, PATCH_SIZE * MAP_SIZE, PATCH_SIZE * MAP_SIZE, 0, GL_RED, GL_FLOAT, &noiseData[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}






	static Shader* mapgen = nullptr;
	static Mesh screenQuad;
	static unsigned int FBO;
	if (!mapgen) {
		mapgen = new Shader;
		mapgen->Load("res/shaders/noise.vertex", GL_VERTEX_SHADER);
		mapgen->Load("res/shaders/noise.fragment", GL_FRAGMENT_SHADER);
		mapgen->Link();
		auto quad = screenQuad.GetVertexData();
		quad->push_back({ {-1, -1, 0} });
		quad->push_back({ {1, 1, 0} });
		quad->push_back({ {-1, 1, 0} });
		quad->push_back({ {1, 1, 0} });
		quad->push_back({ {-1, -1, 0} });
		quad->push_back({ {1, -1, 0} });
		screenQuad.Buffer();
		glGenFramebuffers(1, &FBO);
	}


	// map gen
	//terrainNoise->
	//glGenTextures(1, &m_mapTexture);
	//glBindTexture(GL_TEXTURE_2D, m_mapTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, MAP_SIZE, MAP_SIZE, 0, GL_RED, GL_FLOAT, nullptr);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	//glBindTexture(GL_TEXTURE_2D, m_mapTexture);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mapTexture, 0);

	//glClear(GL_COLOR_BUFFER_BIT);
	//mapgen->Use();
	//screenQuad.Draw(GL_TRIANGLES);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

TerrainPatch::~TerrainPatch() {
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteTextures(1, &m_mapTexture);
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
	m_patchRotation = glm::mat4(1);
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
	glActiveTexture(GL_TEXTURE0 + 6);
	glBindTexture(GL_TEXTURE_2D, m_mapTexture);
	shader->uniform1i("u_noisemap", 6);
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

	position = glm::dvec4(center, 1);
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

bool TerrainPatch::sm_topologiesCreated = false;
TerrainTopology* TerrainPatch::sm_topology[MAX_DETAIL_DIFFERENCE][MAX_DETAIL_DIFFERENCE][MAX_DETAIL_DIFFERENCE][MAX_DETAIL_DIFFERENCE] = { nullptr };