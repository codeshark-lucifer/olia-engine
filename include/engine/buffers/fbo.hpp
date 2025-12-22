#pragma once
#include <glad/glad.h>

class FBO
{
public:
    FBO(int w, int h, int samples)
        : width(w), height(h), samples(samples)
    {
        glGenFramebuffers(1, &ID);
        glBindFramebuffer(GL_FRAMEBUFFER, ID);

        glGenTextures(1, &color);

        if (samples > 1)
        {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                samples, GL_RGB, w, h, GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D_MULTISAMPLE, color, 0);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, color);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, color, 0);
        }

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);

        if (samples > 1)
            glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                samples, GL_DEPTH24_STENCIL8, w, h);
        else
            glRenderbufferStorage(GL_RENDERBUFFER,
                GL_DEPTH24_STENCIL8, w, h);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glViewport(0, 0, width, height);
    }

    void Unbind(int w, int h) const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
    }

    void OnResize(int w, int h)
    {
        width = w;
        height = h;

        if (samples > 1)
        {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                samples, GL_RGB, w, h, GL_TRUE);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, color);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        }

        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        if (samples > 1)
            glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                samples, GL_DEPTH24_STENCIL8, w, h);
        else
            glRenderbufferStorage(GL_RENDERBUFFER,
                GL_DEPTH24_STENCIL8, w, h);
    }

    unsigned int GetID() const { return ID; }
    unsigned int GetTexture() const { return color; }

private:
    int width, height, samples;
    unsigned int ID = 0, color = 0, rbo = 0;
};
