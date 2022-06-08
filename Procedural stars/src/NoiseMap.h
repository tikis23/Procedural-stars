#pragma once
#include <cstdint>
#include <string>
#include "Shader.h"

class NoiseMap {
public:
	NoiseMap(std::uint32_t width, std::uint32_t height);
	~NoiseMap();
	void BindNoise(Shader* shader, const std::string& location, int pos);
private:
	unsigned int m_tex;
};