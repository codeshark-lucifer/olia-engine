#pragma once
#include <glad/glad.h>
#include <stdexcept>

class FBO
{
public:
    FBO(int width, int height)
        : width(width), height(height)
    {
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        // Color texture
        glGenTextures(1, &colorTexture);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB,
            width, height, 0,
            GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            colorTexture,
            0);

        // Depth + Stencil RBO
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH24_STENCIL8,
            width, height);
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Framebuffer not complete");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~FBO()
    {
        glDeleteFramebuffers(1, &id);
        glDeleteTextures(1, &colorTexture);
        glDeleteRenderbuffers(1, &rbo);
    }

    void Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    static void Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    unsigned int GetID() { return id; }
    unsigned int GetTexture() const { return colorTexture; }

private:
    unsigned int id = 0;
    unsigned int colorTexture = 0;
    unsigned int rbo = 0;
    int width, height;
};
