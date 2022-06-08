#include "NoiseMap.h"
#include <GL/glew.h>
#include <FastNoise/FastNoise.h>
#include <vector>
#include "Timer.h"

NoiseMap::NoiseMap(std::uint32_t width, std::uint32_t height) {
    Timer timer;
    // gen noise
    std::vector<float> noise(width * height);
    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
    fnFractal->SetSource(fnSimplex);
    fnFractal->SetOctaveCount(5);

    fnSimplex->GenTileable2D(noise.data(), width, height, 1.f, 1337);
    timer.PrintElapsed("Noise Generated");
    timer.Reset();
    // gen texture
    glGenTextures(1, &m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, &noise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    timer.PrintElapsed("Texture Generated");
}

NoiseMap::~NoiseMap() {
    glDeleteTextures(1, &m_tex);
}

void NoiseMap::BindNoise(Shader* shader, const std::string& location, int pos) {
    glActiveTexture(GL_TEXTURE0 + pos);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    shader->uniform1i(location, pos);
}