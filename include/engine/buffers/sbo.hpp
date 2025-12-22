#pragma once
#include <glad/glad.h>

class SBO
{
public:
    SBO(int w, int h) : width(w), height(h)
    {
        glGenFramebuffers(1, &ID);
        glBindFramebuffer(GL_FRAMEBUFFER, ID);

        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float border[] = {1,1,1,1};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, depth, 0);

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glViewport(0, 0, width, height);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    void Unbind(int w, int h) const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
    }

    unsigned int GetDepthTexture() const { return depth; }

private:
    int width, height;
    unsigned int ID = 0;
    unsigned int depth = 0;
};
