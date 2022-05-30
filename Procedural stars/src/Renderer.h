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
	std::shared_ptr<Shader> m_lightingShader;
	std::shared_ptr<Shader> m_debugShader;
	bool m_smoothShading = true;
	bool m_backFaceCulling = true;
	bool m_showWireframe = false;

	bool m_debugMode = false;
	struct DEBUG_VARS {
		bool color = false;
		bool normals = false;
		bool diffuse = false;
		bool specular = false;
	}debugvars;
};