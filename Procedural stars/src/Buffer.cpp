#include "Buffer.h"
#include <GL/glew.h>
#include <iostream>
#include <random>
#include "glm/gtc/type_ptr.hpp"

float lerp(float a, float b, float f) {
    return a + f * (b - a);
}

GBuffer::GBuffer(std::uint32_t width, std::uint32_t height) {
    Create(width, height);
}

GBuffer::~GBuffer() {
    Delete();
}

void GBuffer::ResizeCallback(std::uint32_t width, std::uint32_t height) {
    Delete();
    Create(width, height);
}

void GBuffer::BindWrite() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GBuffer::BindRead() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GBuffer::BindColor(Shader* shader, int pos) {
    glActiveTexture(GL_TEXTURE0 + pos);
    glBindTexture(GL_TEXTURE_2D, m_texColor);
    shader->uniform1i("u_texColor", pos);
}

void GBuffer::BindPosition(Shader* shader, int pos) {
    glActiveTexture(GL_TEXTURE0 + pos);
    glBindTexture(GL_TEXTURE_2D, m_texPosition);
    shader->uniform1i("u_texPosition", pos);
}
void GBuffer::BindNormal(Shader* shader, int pos) {
    glActiveTexture(GL_TEXTURE0 + pos);
    glBindTexture(GL_TEXTURE_2D, m_texNormal);
    shader->uniform1i("u_texNormal", pos);
}


void GBuffer::Create(std::uint32_t width, std::uint32_t height) {
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_texColor);
    glBindTexture(GL_TEXTURE_2D, m_texColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texColor, 0);

    glGenTextures(1, &m_texPosition);
    glBindTexture(GL_TEXTURE_2D, m_texPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_texPosition, 0);

    glGenTextures(1, &m_texNormal);
    glBindTexture(GL_TEXTURE_2D, m_texNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_texNormal, 0);

    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &m_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

    //Check Status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "error while initializing framebuffer" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << '\n';
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::Delete() {
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1, &m_texColor);
    glDeleteTextures(1, &m_texPosition);
    glDeleteTextures(1, &m_texNormal);
    glDeleteRenderbuffers(1, &m_depth);
    m_fbo = 0;
    m_depth = 0;
    m_texColor = 0;
    m_texPosition = 0;
    m_texNormal = 0;
}

SSAOBuffer::SSAOBuffer(std::uint32_t width, std::uint32_t height) {
    Create(width, height);
}

SSAOBuffer::~SSAOBuffer() {
    Delete();
}

void SSAOBuffer::ResizeCallback(std::uint32_t width, std::uint32_t height) {
    Delete();
    Create(width, height);
}

void SSAOBuffer::BindWrite() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
}

void SSAOBuffer::BindRead() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAOBuffer::BindNoise(Shader* shader, int pos) {
    glActiveTexture(GL_TEXTURE0 + pos);
    glBindTexture(GL_TEXTURE_2D, m_texNoise);
    shader->uniform1i("u_texNormal", pos);
}

void SSAOBuffer::BindSSAO(Shader* shader, int pos) {
    glActiveTexture(GL_TEXTURE0 + pos);
    glBindTexture(GL_TEXTURE_2D, m_ssao);
    shader->uniform1i("u_texSSAO", pos);
}

void SSAOBuffer::BindKernel(Shader* shader) {
    shader->uniformArr3f("u_samples", m_kernels.size(), glm::value_ptr(m_kernels[0]));
}

void SSAOBuffer::Create(std::uint32_t width, std::uint32_t height) {
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_ssao);
    glBindTexture(GL_TEXTURE_2D, m_ssao);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssao, 0);

    // kernels
    std::uniform_real_distribution<float> randomFloat(0.0, 1.0);
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloat(generator) * 2.0 - 1.0, randomFloat(generator) * 2.0 - 1.0, randomFloat(generator));
        sample = glm::normalize(sample);
        sample *= randomFloat(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        m_kernels.push_back(sample);
    }

    // noise
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(
            randomFloat(generator) * 2.0 - 1.0,
            randomFloat(generator) * 2.0 - 1.0,
            0.0f);
        m_noise.push_back(noise);
    }

    // generate noise texture
    glGenTextures(1, &m_texNoise);
    glBindTexture(GL_TEXTURE_2D, m_texNoise);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &m_noise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //Check Status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "error while initializing framebuffer" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << '\n';
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAOBuffer::Delete() {
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1, &m_ssao);
    glDeleteTextures(1, &m_texNoise);
    m_fbo = 0;
    m_ssao = 0;
    m_texNoise = 0;
    m_kernels.clear();
    m_noise.clear();
}
