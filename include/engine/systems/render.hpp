#pragma once
#include <engine/systems/system.hpp>
#include <engine/ec/entity.hpp>
#include <engine/shader.hpp>

#include <engine/buffers/fbo.hpp>
#include <engine/buffers/sbo.hpp>
#include <engine/buffers/quad.hpp>

#include <engine/components/camera.hpp>
#include <engine/components/meshrenderer.hpp>
#include <engine/config.h>

class RenderSystem : public System
{
public:
    RenderSystem(int w, int h)
        : width(w), height(h)
    {
        fbo  = std::make_unique<FBO>(w, h, MSAASAMPLES); // MSAA
        ifbo = std::make_unique<FBO>(w, h, 1);          // resolved

        sbo = std::make_unique<SBO>(SHADOW_WIDHT, SHADOW_HEIGHT);
        screen = std::make_unique<Quad>();
        camera = std::make_unique<Camera>(w, h);

        defaultShader = std::make_unique<Shader>("assets/shaders/glsl/default.glsl");
        depthShader   = std::make_unique<Shader>("assets/shaders/glsl/depth.glsl");
        bufferShader  = std::make_unique<Shader>(
            "assets/shaders/vert/render.vert",
            "assets/shaders/frag/fbo.frag"
        );
    }

    void Update(const std::vector<std::shared_ptr<Entity>>& entities, float) override
    {
        RenderShadowPass(entities);
        RenderScenePass(entities);
        ResolveMSAA();
        RenderScreen();
    }

    void OnResize(int w, int h)
    {
        width = w;
        height = h;

        glViewport(0, 0, w, h);
        glScissor(0, 0, w, h);

        fbo->OnResize(w, h);
        ifbo->OnResize(w, h);
        camera->OnResize(w, h);
    }

private:
    int width = 0, height = 0;
    glm::mat4 lightSpaceMatrix{1.0f};

    std::unique_ptr<FBO> fbo, ifbo;
    std::unique_ptr<SBO> sbo;
    std::unique_ptr<Quad> screen;
    std::unique_ptr<Camera> camera;

    std::unique_ptr<Shader> defaultShader;
    std::unique_ptr<Shader> depthShader;
    std::unique_ptr<Shader> bufferShader;

private:
    // ---------------- PASSES ----------------

    void RenderShadowPass(const std::vector<std::shared_ptr<Entity>>& entities)
    {
        glm::mat4 proj = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 25.f);
        glm::mat4 view = glm::lookAt(
            glm::vec3(0, 3, -5),
            glm::vec3(0),
            glm::vec3(0, 1, 0)
        );

        lightSpaceMatrix = proj * view;

        sbo->Bind();

        depthShader->Use();
        depthShader->SetUniform("lightSpaceMatrix", lightSpaceMatrix);

        Draw(entities, *depthShader);

        sbo->Unbind(width, height);
    }

    void RenderScenePass(const std::vector<std::shared_ptr<Entity>>& entities)
    {
        fbo->Bind();

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        defaultShader->Use();
        defaultShader->SetUniform("view", camera->GetView());
        defaultShader->SetUniform("projection", camera->GetProjection());
        defaultShader->SetUniform("viewPos", camera->GetPosition());
        defaultShader->SetUniform("lightPos", glm::vec3(0, 3, -5));
        defaultShader->SetUniform("lightColor", glm::vec3(1));
        defaultShader->SetUniform("lightSpaceMatrix", lightSpaceMatrix);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sbo->GetDepthTexture());
        defaultShader->SetUniform("shadowMap", 1);

        Draw(entities, *defaultShader);

        fbo->Unbind(width, height);
    }

    void ResolveMSAA()
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->GetID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ifbo->GetID());

        glBlitFramebuffer(
            0, 0, width, height,
            0, 0, width, height,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void RenderScreen()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glScissor(0, 0, width, height);

        glDisable(GL_SCISSOR_TEST);   // 🔥 CRITICAL
        glDisable(GL_DEPTH_TEST);

        screen->Draw(*bufferShader, ifbo->GetTexture());

        glEnable(GL_DEPTH_TEST);
    }

    // ---------------- DRAW ----------------

    void Draw(const std::vector<std::shared_ptr<Entity>>& entities, Shader& shader)
    {
        for (auto& e : entities)
            DrawRecursive(e, shader);
    }

    void DrawRecursive(const std::shared_ptr<Entity>& e, Shader& shader)
    {
        for (auto& m : e->GetComponents<MeshRenderer>())
            m->Render(shader);

        for (auto& c : e->children)
            DrawRecursive(c, shader);
    }
};
