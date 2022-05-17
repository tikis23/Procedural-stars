#pragma once
#include <cstdint>
#include "Shader.h"
class GBuffer {
public:
	GBuffer(std::uint32_t width, std::uint32_t height);
	~GBuffer();
	void BindWrite();
	void BindRead(Shader* shader);
private:
	unsigned int m_fbo;
	unsigned int m_depth;
	unsigned int m_texColor;
	unsigned int m_texPosition;
	unsigned int m_texNormal;
};