#include "Mesh.h"

Mesh::Mesh() {
	glGenBuffers(1, &m_VBO);
	glGenVertexArrays(1, &m_VAO);
}

Mesh::~Mesh() {
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
}

void Mesh::Draw() {
	if (m_buffered) {
		glBindVertexArray(m_VAO);
		glDrawArrays(GL_TRIANGLES, 0, m_vertexData.size());
		TriangleCount += m_vertexData.size() / 3;
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
	m_buffered = true;
}

int Mesh::TriangleCount = 0;