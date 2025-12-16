#pragma once
#include <glad/glad.h>
#include <stdexcept>

class FBO
{
public:
    FBO(int w, int h, int s = 1)
        : width(w), height(h), samples(s)
    {
        // Generate framebuffer
        glGenFramebuffers(1, &ID);
        Bind();

        // --- Create color texture ---
        glGenTextures(1, &texture);
        if (samples > 1) {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture, 0);
        } else {
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        }

        // --- Create depth + stencil renderbuffer ---
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        if (samples > 1) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        }
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        // --- Check completeness ---
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Framebuffer is not complete!");

        // Unbind to avoid accidental rendering
        Unbind();
    }

    ~FBO()
    {
        glDeleteFramebuffers(1, &ID);
        glDeleteTextures(1, &texture);
        glDeleteRenderbuffers(1, &rbo);
    }

    void Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glViewport(0, 0, width, height);
    }

    void Unbind(int screenWidth = 0, int screenHeight = 0) const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (screenWidth > 0 && screenHeight > 0)
            glViewport(0, 0, screenWidth, screenHeight);
    }

    unsigned int GetTexture() const { return ID; }
    unsigned int GetColorAttachmentTexture() const { return texture; } // New method

    void OnResize(int w, int h)
    {
        if (width == w && height == h)
            return;

        width = w;
        height = h;

        // Resize color texture
        if (samples > 1) {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
        } else {
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        }

        // Resize renderbuffer
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        if (samples > 1) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
        } else {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        }
        
        // Optional: unbind
        if (samples > 1) {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    void Clear(float r = 0.0f, float g = 0.0f, float b = 0.0f)
    {
        Bind();
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        Unbind();
    }

private:
    int width = 0, height = 0;
    unsigned int ID = 0;
    unsigned int rbo = 0;
    unsigned int texture = 0;
    int samples = 0;
};
