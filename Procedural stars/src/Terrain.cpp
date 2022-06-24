#include "Terrain.h"
#include "Timer.h"
Terrain::Terrain() {
	for (int i = 0; i < 6; i++) {
		m_face[i] = new TerrainFace(i);
	}

	//m_face[FACE_TOP   ]->Connect(NORTH, m_face[FACE_BACK]);
	//m_face[FACE_BACK  ]->Connect(NORTH, m_face[FACE_BOTTOM]);
	//m_face[FACE_BOTTOM]->Connect(NORTH, m_face[FACE_FRONT]);
	//m_face[FACE_FRONT ]->Connect(NORTH, m_face[FACE_TOP]);
	//
	//m_face[FACE_TOP   ]->Connect(SOUTH, m_face[FACE_FRONT]);
	//m_face[FACE_BACK  ]->Connect(SOUTH, m_face[FACE_TOP]);
	//m_face[FACE_BOTTOM]->Connect(SOUTH, m_face[FACE_BACK]);
	//m_face[FACE_FRONT ]->Connect(SOUTH, m_face[FACE_BOTTOM]);

	//m_face[FACE_TOP   ]->Connect(EAST, m_face[FACE_RIGHT]);
	//m_face[FACE_TOP   ]->Connect(WEST, m_face[FACE_LEFT]);
	//m_face[FACE_BACK  ]->Connect(EAST, m_face[FACE_RIGHT]);
	//m_face[FACE_BACK  ]->Connect(WEST, m_face[FACE_LEFT]);
	//m_face[FACE_BOTTOM]->Connect(EAST, m_face[FACE_RIGHT]);
	//m_face[FACE_BOTTOM]->Connect(WEST, m_face[FACE_LEFT]);
	//m_face[FACE_FRONT ]->Connect(EAST, m_face[FACE_RIGHT]);
	//m_face[FACE_FRONT ]->Connect(WEST, m_face[FACE_LEFT]);


	//m_face[FACE_FRONT]->Connect(NORTH, m_face[FACE_TOP]);
	//m_face[FACE_FRONT]->Connect(WEST , m_face[FACE_LEFT]);
	//m_face[FACE_FRONT]->Connect(SOUTH, m_face[FACE_BOTTOM]);
	//m_face[FACE_LEFT ]->Connect(NORTH, m_face[FACE_TOP]);
	//m_face[FACE_LEFT ]->Connect(WEST , m_face[FACE_BACK]);
	//m_face[FACE_LEFT ]->Connect(SOUTH, m_face[FACE_BOTTOM]);
	//m_face[FACE_BACK ]->Connect(NORTH, m_face[FACE_TOP]);
	//m_face[FACE_BACK ]->Connect(WEST , m_face[FACE_RIGHT]);
	//m_face[FACE_BACK ]->Connect(SOUTH, m_face[FACE_BOTTOM]);
	//m_face[FACE_RIGHT]->Connect(NORTH, m_face[FACE_TOP]);
	//m_face[FACE_RIGHT]->Connect(WEST , m_face[FACE_FRONT]);
	//m_face[FACE_RIGHT]->Connect(SOUTH, m_face[FACE_BOTTOM]);
}

Terrain::~Terrain() {
}

void Terrain::Update(glm::dvec3 cameraPos) {
	for (int i = 0; i < 6; i++) {
		m_face[i]->Update(cameraPos);
	}
	QuadTreeNode::sm_splitsLeft = MAX_SPLITS;
}

void Terrain::Render(Shader* shader) {
	for (int i = 0; i < 6; i++) {
		m_face[i]->Render(shader);
	}
}