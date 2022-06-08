#pragma once
#include "Shader.h"
#include "Camera.h"
#include "Window.h"
#include "Buffer.h"

class Renderer {
public:
	Renderer(Window* window);
	~Renderer();
	void Draw(Camera* cam, Window* window);
private:
	void LoadShaders();

	GBuffer m_gbuffer;
	SSAOBuffer m_ssaobuffer;

	std::shared_ptr<Shader> m_terrainShader;
	std::shared_ptr<Shader> m_lightingShader;
	std::shared_ptr<Shader> m_SSAOShader;
	bool m_backFaceCulling = true;
	bool m_ssao = true;
	bool m_showWireframe = false;
	bool m_showLod = false;
	struct DEBUG_VARS {
		bool color = false;
		bool normals = false;
		bool diffuse = false;
		bool specular = false;
		bool ssao = false;
	}debugvars;
};