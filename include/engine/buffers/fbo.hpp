#pragma once
#include <glad/glad.h>
#include <stdexcept>

class FBO
{
public:
    // width, height, MSAA samples (1 = no MSAA)
    explicit FBO(int width, int height, int samples = 1)
        : m_width(width), m_height(height), m_samples(samples)
    {
        Create();
    }

    // Non-copyable (OpenGL resource ownership)
    FBO(const FBO&) = delete;
    FBO& operator=(const FBO&) = delete;

    // Movable
    FBO(FBO&& other) noexcept { MoveFrom(other); }
    FBO& operator=(FBO&& other) noexcept
    {
        if (this != &other)
        {
            Delete();
            MoveFrom(other);
        }
        return *this;
    }

    ~FBO()
    {
        Delete();
    }

    void Resize(int width, int height)
    {
        if (width == m_width && height == m_height)
            return;

        m_width = width;
        m_height = height;
        Delete();
        Create();
    }

    void Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
        glViewport(0, 0, m_width, m_height);
    }

    static void Unbind(int w, int h)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0,0, w, h);
    }

    unsigned int GetID() const { return m_fboID; }
    unsigned int GetColorTexture() const { return m_colorTex; }
    bool IsMSAA() const { return m_samples > 1; }

    // Resolve MSAA into another FBO (or default framebuffer if target ID = 0)
    void BlitTo(const FBO& target) const
    {
        if (!IsMSAA())
            return;

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.GetID());

        glBlitFramebuffer(
            0, 0, m_width, m_height,
            0, 0, target.m_width, target.m_height,
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
            GL_NEAREST
        );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

private:
    unsigned int m_fboID    = 0;
    unsigned int m_colorTex = 0;
    unsigned int m_rbo      = 0;

    int m_width   = 0;
    int m_height  = 0;
    int m_samples = 1;

private:
    void Create()
    {
        glGenFramebuffers(1, &m_fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

        if (IsMSAA())
        {
            // MSAA color buffer
            glGenTextures(1, &m_colorTex);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_colorTex);
            glTexImage2DMultisample(
                GL_TEXTURE_2D_MULTISAMPLE,
                m_samples,
                GL_RGBA8,
                m_width,
                m_height,
                GL_TRUE
            );
            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D_MULTISAMPLE,
                m_colorTex,
                0
            );

            // MSAA depth + stencil
            glGenRenderbuffers(1, &m_rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
            glRenderbufferStorageMultisample(
                GL_RENDERBUFFER,
                m_samples,
                GL_DEPTH24_STENCIL8,
                m_width,
                m_height
            );
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER,
                GL_DEPTH_STENCIL_ATTACHMENT,
                GL_RENDERBUFFER,
                m_rbo
            );
        }
        else
        {
            // Color texture
            glGenTextures(1, &m_colorTex);
            glBindTexture(GL_TEXTURE_2D, m_colorTex);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA8,
                m_width,
                m_height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                nullptr
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D,
                m_colorTex,
                0
            );

            // Depth + stencil
            glGenRenderbuffers(1, &m_rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
            glRenderbufferStorage(
                GL_RENDERBUFFER,
                GL_DEPTH24_STENCIL8,
                m_width,
                m_height
            );
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER,
                GL_DEPTH_STENCIL_ATTACHMENT,
                GL_RENDERBUFFER,
                m_rbo
            );
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            throw std::runtime_error(IsMSAA() ? "MSAA FBO incomplete" : "FBO incomplete");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Delete()
    {
        if (m_rbo)
            glDeleteRenderbuffers(1, &m_rbo);
        if (m_colorTex)
            glDeleteTextures(1, &m_colorTex);
        if (m_fboID)
            glDeleteFramebuffers(1, &m_fboID);

        m_rbo = m_colorTex = m_fboID = 0;
    }

    void MoveFrom(FBO& other)
    {
        m_fboID    = other.m_fboID;
        m_colorTex = other.m_colorTex;
        m_rbo      = other.m_rbo;
        m_width    = other.m_width;
        m_height   = other.m_height;
        m_samples  = other.m_samples;

        other.m_fboID = other.m_colorTex = other.m_rbo = 0;
    }
};
