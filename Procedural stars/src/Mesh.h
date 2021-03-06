#pragma once
#include <GL/glew.h>
#include <vector>
#include "glm/glm.hpp"

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 normal;
	int edge;
};

class Mesh {
public:
	Mesh();
	~Mesh();
	void Draw(GLenum mode);
	std::vector<Vertex>* GetVertexData() { return &m_vertexData; };
	void Buffer();
	bool IsBuffered() { return m_buffered; };
	void IsBuffered(bool val) { m_buffered = val; };
	bool NeedUnmap() { return m_unmap; };
	void NeedUnmap(bool val) { m_unmap = val; };
	void Allocate(size_t size);
	void* MapBuffer();
	void UnmapBuffer();
	void ClearBufferData();
	void ClearVertexData();
	static int VertexCount;
private:
	std::vector<Vertex> m_vertexData;
	int m_drawSize;
	unsigned int m_VAO;
	unsigned int m_VBO;
	bool m_buffered = false;
	bool m_unmap = false;
};