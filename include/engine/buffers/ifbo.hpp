#pragma once
#include <glad/glad.h>
#include <stdexcept>

class IFBO
{
public:
    IFBO(int width, int height, int samples = 4)
        : width(width), height(height), samples(samples)
    {
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        // Multisample color buffer
        glGenTextures(1, &colorMS);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorMS);
        glTexImage2DMultisample(
            GL_TEXTURE_2D_MULTISAMPLE,
            samples,
            GL_RGB,
            width,
            height,
            GL_TRUE
        );
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D_MULTISAMPLE,
            colorMS,
            0
        );

        // Multisample depth-stencil
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            samples,
            GL_DEPTH24_STENCIL8,
            width,
            height
        );
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            rbo
        );

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("MSAA framebuffer incomplete");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~IFBO()
    {
        glDeleteFramebuffers(1, &id);
        glDeleteTextures(1, &colorMS);
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

    unsigned int GetID() const { return id; }

private:
    unsigned int id = 0;
    unsigned int colorMS = 0;
    unsigned int rbo = 0;
    int width, height, samples;
};
