#pragma once
#include <glad/glad.h>
#include <stdexcept>

class ShadowBuffer
{
public:
    ShadowBuffer(int w, int h)
        : width(w), height(h)
    {
        // Create framebuffer
        glGenFramebuffers(1, &ID);
        glBindFramebuffer(GL_FRAMEBUFFER, ID);

        // Create depth texture
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // Important: border color for shadows outside the frustum
        float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // Attach depth texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

        // No color buffer needed
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Shadow FBO is not complete!");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~ShadowBuffer()
    {
        glDeleteFramebuffers(1, &ID);
        glDeleteTextures(1, &depthTexture);
    }

    void Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glViewport(0, 0, width, height);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void Unbind(int screenWidth = 0, int screenHeight = 0) const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (screenWidth > 0 && screenHeight > 0)
            glViewport(0, 0, screenWidth, screenHeight);
    }

    unsigned int GetDepthTexture() const { return depthTexture; }

private:
    int width, height;
    unsigned int ID = 0;
    unsigned int depthTexture = 0;
};
