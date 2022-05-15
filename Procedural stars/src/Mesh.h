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
	void Buffer();
	std::vector<Vertex>* GetVertexData() { return &m_vertexData; };
private:
	std::vector<Vertex> m_vertexData;
	unsigned int m_VAO;
	unsigned int m_VBO;
};