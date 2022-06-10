#include "Terrain.h"

Terrain::Terrain() {
	for (int i = 0; i < 6; i++) {
		m_face[i] = new TerrainFace(i);
	}
}

Terrain::~Terrain() {
}

void Terrain::Update() {
	for (int i = 0; i < 6; i++) {
		m_face[i]->Update();
	}
}

void Terrain::Render(Shader* shader) {
	for (int i = 0; i < 6; i++) {
		m_face[i]->Render(shader);
	}
}
