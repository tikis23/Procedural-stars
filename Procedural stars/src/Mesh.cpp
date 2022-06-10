#include "Mesh.h"

Mesh::Mesh() {
	glGenBuffers(1, &m_VBO);
	glGenVertexArrays(1, &m_VAO);
}

Mesh::~Mesh() {
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void Mesh::Draw(GLenum mode) {
	if (m_buffered) {
		glBindVertexArray(m_VAO);
		glDrawArrays(mode, 0, m_drawSize);
		VertexCount += m_drawSize;
	}
}

void Mesh::Buffer() {
	if (m_vertexData.empty())
		return;
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(Vertex), &m_vertexData[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, edge));
	glEnableVertexAttribArray(3);
	m_drawSize = m_vertexData.size();
	m_buffered = true;
}

void Mesh::Allocate(size_t size) {
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glBufferData(GL_ARRAY_BUFFER, size * sizeof(Vertex), 0, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, edge));
	glEnableVertexAttribArray(3);
	m_drawSize = size;
}

void* Mesh::MapBuffer() {
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	return glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void Mesh::UnmapBuffer() {
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
}

void Mesh::ClearBufferData() {
	m_buffered = false;
	glDeleteBuffers(1, &m_VBO);
	glGenBuffers(1, &m_VBO);
	m_drawSize = 0;
}

void Mesh::ClearVertexData() {
	m_vertexData.clear();
}

int Mesh::VertexCount = 0;