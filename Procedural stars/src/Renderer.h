#pragma once

#include "Shader.h"
#include "Camera.h"

class Renderer {
public:
	Renderer();
	~Renderer();
	void Draw(Camera* cam);
private:
	void LoadShaders();

	std::shared_ptr<Shader> m_mainShader;

	bool m_showNormals = false;
	bool m_showWireframe = false;
	bool m_smoothShading = true;
	bool m_backFaceCulling = false;
};