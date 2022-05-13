#pragma once

#include "Shader.h"
#include "Camera.h"

class Renderer {
public:
	Renderer();
	~Renderer();
	void Draw(Camera* cam);
private:
	std::shared_ptr<Shader> m_mainShader;
};