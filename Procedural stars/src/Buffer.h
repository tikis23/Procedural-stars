#pragma once
#include <cstdint>
#include <vector>
#include "Shader.h"

class GBuffer {
public:
	GBuffer(std::uint32_t width, std::uint32_t height);
	~GBuffer();
	void BindWrite();
	void BindRead();
	void BindColor(Shader* shader, int pos);
	void BindNormal(Shader* shader, int pos);
	void BindPosition(Shader* shader, int pos);
private:
	unsigned int m_fbo;
	unsigned int m_depth;
	unsigned int m_texColor;
	unsigned int m_texPosition;
	unsigned int m_texNormal;
};

class SSAOBuffer {
public:
	SSAOBuffer(std::uint32_t width, std::uint32_t height);
	~SSAOBuffer();
	void BindWrite();
	void BindRead();
	void BindNoise(Shader* shader, int pos);
	void BindSSAO(Shader* shader, int pos);
	void BindKernel(Shader* shader);
private:
	unsigned int m_fbo;
	unsigned int m_ssao;
	unsigned int m_texNoise;
	std::vector<glm::vec3> m_kernels;
	std::vector<glm::vec3> m_noise;
};