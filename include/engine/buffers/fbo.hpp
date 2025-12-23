#pragma once
#include <glad/glad.h>
#include <stdexcept>
#include <iostream>

class FBO
{
public:
    // Constructor: width, height, MSAA samples (1 = no MSAA)
    FBO(int width, int height, int samples = 1)
        : width(width), height(height), samples(samples)
    {
        Create();
    }

    ~FBO()
    {
        Delete();
    }

    // Recreate framebuffer (used on resize)
    void Resize(int newWidth, int newHeight)
    {
        width = newWidth;
        height = newHeight;
        Delete();
        Create();
    }

    // Bind FBO for rendering
    void Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);
        glViewport(0, 0, width, height);
    }

    // Unbind FBO (bind default framebuffer)
    static void Unbind(int w, int h)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
    }

    // Getters
    unsigned int GetColorTexture() const { return colorTex; }
    unsigned int GetID() const { return fboID; }
    bool IsMSAA() const { return samples > 1; }

    // Blit MSAA FBO to another FBO (or default)
    void BlitTo(const FBO &target) const
    {
        if (!IsMSAA()) return;

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.GetID());
        glBlitFramebuffer(
            0, 0, width, height,
            0, 0, target.width, target.height,
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
            GL_NEAREST
        );
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

private:
    unsigned int fboID = 0;
    unsigned int colorTex = 0;
    unsigned int rbo = 0;
    int width, height;
    int samples = 1;

    void Create()
    {
        glGenFramebuffers(1, &fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);

        if (samples > 1)
        {
            // MSAA color buffer
            glGenTextures(1, &colorTex);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorTex);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorTex, 0);

            // MSAA depth+stencil RBO
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        }
        else
        {
            // Regular color texture
            glGenTextures(1, &colorTex);
            glBindTexture(GL_TEXTURE_2D, colorTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

            // Depth+stencil RBO
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error(samples > 1 ? "MSAA FBO incomplete" : "FBO incomplete");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Delete()
    {
        if (rbo) { glDeleteRenderbuffers(1, &rbo); rbo = 0; }
        if (colorTex) { glDeleteTextures(1, &colorTex); colorTex = 0; }
        if (fboID) { glDeleteFramebuffers(1, &fboID); fboID = 0; }
    }
};
