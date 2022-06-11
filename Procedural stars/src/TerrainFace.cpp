#include "TerrainFace.h"
#include "glm/gtx/transform.hpp"
TerrainFace::TerrainFace(int face) :
	m_face(face), m_rootNode(new QuadTreeNode) {
	m_rootNode->m_face = face;
	m_rootNode->m_depth = 0;
	m_rootNode->m_parent = nullptr;
	m_rootNode->m_position = { 0, 0 };
	m_rootNode->m_size = 1.0 / (1 << 0);
	switch (face)
	{
	case FACE_TOP:
		m_rootNode->m_patchRotation = glm::rotate(glm::mat4(1), glm::radians(0.f), glm::vec3(0, 1, 0));
		break;
	case FACE_BOTTOM:
		m_rootNode->m_patchRotation = glm::rotate(glm::mat4(1), glm::radians(180.f), glm::vec3(0, 0, 1));
		break;
	case FACE_LEFT:
		m_rootNode->m_patchRotation = glm::rotate(glm::mat4(1), glm::radians(90.f), glm::vec3(1, 0, 0));
		break;
	case FACE_RIGHT:
		m_rootNode->m_patchRotation = glm::rotate(glm::mat4(1), glm::radians(-90.f), glm::vec3(1, 0, 0));
		break;
	case FACE_FRONT:
		m_rootNode->m_patchRotation = glm::rotate(glm::mat4(1), glm::radians(90.f), glm::vec3(0, 0, 1));
		break;
	case FACE_BACK:
		m_rootNode->m_patchRotation = glm::rotate(glm::mat4(1), glm::radians(-90.f), glm::vec3(0, 0, 1));
		break;
	default:
		break;
	}
}

TerrainFace::~TerrainFace() {
}

void TerrainFace::Update(glm::dvec3 cameraPos) {
	if (m_rootNode != nullptr)
		m_rootNode->Update(cameraPos);
}

void TerrainFace::Render(Shader* shader) {
	m_rootNode->Render(shader);
}

void TerrainFace::Connect(const unsigned int side, TerrainFace* face) {
	// Connect the face top nodes
	if (m_rootNode && face->m_rootNode)
		m_rootNode->SetNeighbor(side, face->m_rootNode);
}