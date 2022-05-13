#pragma once
#include <GL/glew.h>
#include <vector>
#include "glm/glm.hpp"

struct Vertex {
	glm::vec3 position;
};

class Mesh {
public:
	Mesh();
	~Mesh();
	void Draw();
private:
	std::vector<Vertex> m_vertexData;
};