#pragma once

#include "TerrainFace.h"
#include "Shader.h"
#include "glm/glm.hpp"

class Terrain {
public:
	Terrain();
	~Terrain();
	void Update(glm::dvec3 cameraPos);
	void Render(Shader* shader);
private:
	TerrainFace* m_face[6] = {nullptr};
};