#pragma once
#include <glad/glad.h>
#include <stdexcept>

class SBO
{
public:
    SBO(int size = 2048)
        : size(size)
    {
        glGenFramebuffers(1, &fbo);

        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_DEPTH_COMPONENT,
            size, size,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            nullptr
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float borderColor[] = { 1,1,1,1 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D,
            depthMap,
            0
        );

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Shadow framebuffer incomplete");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~SBO()
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &depthMap);
    }

    void Bind() const
    {
        glViewport(0, 0, size, size);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
    }

    static void Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    unsigned int GetDepthMap() const { return depthMap; }

private:
    unsigned int fbo = 0;
    unsigned int depthMap = 0;
    int size;
};
