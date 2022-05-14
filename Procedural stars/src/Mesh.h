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
	glm::mat4& GetModelMatrix() { return m_model; };
	void SetModelMatrix(glm::mat4 model) { m_model = model; };
	std::vector<Vertex>* GetVertexData() { return &m_vertexData; };
private:
	std::vector<Vertex> m_vertexData;
	glm::mat4 m_model;
	unsigned int m_VAO;
	unsigned int m_VBO;
};