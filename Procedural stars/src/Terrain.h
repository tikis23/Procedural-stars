#pragma once

#include "TerrainFace.h"
#include "Shader.h"

class Terrain {
public:
	Terrain();
	~Terrain();
	void Update();
	void Render(Shader* shader);
private:
	TerrainFace* m_face[6] = {nullptr};
};