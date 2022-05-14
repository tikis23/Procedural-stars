#pragma once

#include <vector>
#include "Mesh.h"
#include "glm/glm.hpp"
#include "Camera.h"

class Planet {
public:
	Planet(Camera* cam);
	~Planet();
	std::vector<Mesh>& GetMeshes() { return m_meshes; };
private:
	std::vector<Mesh> m_meshes;
};