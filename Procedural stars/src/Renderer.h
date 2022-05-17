#pragma once

#include "Shader.h"
#include "Camera.h"
#include "Window.h"
class Renderer {
public:
	Renderer();
	~Renderer();
	void Draw(Camera* cam, Window* window);
private:
	void LoadShaders();

	std::shared_ptr<Shader> m_terrainShader;
	std::shared_ptr<Shader> m_oceanShader;
	std::shared_ptr<Shader> m_lightingShader;

	bool m_showNormals = false;
	bool m_showWireframe = false;
	bool m_smoothShading = true;
	bool m_backFaceCulling = false;
};