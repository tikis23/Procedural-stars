#pragma once
#include <GL/glew.h>
#include <vector>
#include "glm/glm.hpp"

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
};

class Mesh {
public:
	Mesh();
	~Mesh();
	void Draw();
	void Buffer();
	std::vector<Vertex>* GetVertexData() { return &m_vertexData; };
	bool IsBuffered() { return m_buffered; };
	bool ReadyToBuffer() { return m_readyToBuffer; };
	void ReadyToBuffer(bool val) { m_readyToBuffer = val; };

	static int TriangleCount;
private:
	std::vector<Vertex> m_vertexData;
	unsigned int m_VAO;
	unsigned int m_VBO;
	bool m_buffered = false;
	bool m_readyToBuffer = false;
};