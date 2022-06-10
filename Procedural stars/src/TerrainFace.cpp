#include "TerrainFace.h"
#include "glm/gtx/transform.hpp"
TerrainFace::TerrainFace(int face) :
	m_face(face), m_rootNode(new QuadTreeNode) {
	m_rootNode->m_depth = 0;
	m_rootNode->m_parent = nullptr;
	m_rootNode->m_position = { 0, 0 };
	m_rootNode->m_size = 1;
	switch (face)
	{
	case FACE_TOP:
		m_rootNode->m_patchRotation = glm::rotate(glm::mat4(1), glm::radians(0.f), glm::vec3(0, 1, 0));
		break;
	case FACE_BOTTOM:
		m_rootNode->m_patchRotation = glm::rotate(glm::mat4(1), glm::radians(180.f), glm::vec3(1, 0, 0));
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

void TerrainFace::Update() {
	m_splitsRemaining = 2;

	if (m_rootNode != nullptr)
		m_rootNode->Update();
}

void TerrainFace::Render(Shader* shader) {
	m_rootNode->m_patch->Render(shader);
}