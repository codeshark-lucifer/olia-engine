#pragma once
#include <engine/systems/system.hpp>
#include <engine/ec/entity.hpp>
#include <engine/shader.hpp>

#include <engine/buffers/fbo.hpp>
#include <engine/buffers/sbo.hpp>
#include <engine/buffers/quad.hpp>

#include <iostream>
#include <engine/components/camera.hpp>
#include <engine/components/meshrenderer.hpp>
#include <engine/config.h>

// handle fbo, sbo, shader
class RenderSystem : public System
{
public:
    RenderSystem(const int &w, const int &h) : width(w), height(h)
    {
        fbo = std::make_unique<FBO>(w, h, MSAASAMPLES);
        ifbo = std::make_unique<FBO>(w, h, 1);
        sbo = std::make_unique<SBO>(SHADOW_WIDHT, SHADOW_HEIGHT);
        screen = std::make_unique<Quad>();
        defaultCamera = std::make_unique<Camera>(w, h);

        defaultShader = std::make_unique<Shader>("assets/shaders/glsl/default.glsl");
        depthShader = std::make_unique<Shader>("assets/shaders/glsl/depth.glsl");
        bufferShader = std::make_unique<Shader>("assets/shaders/vert/render.vert", "assets/shaders/frag/fbo.frag");
    }

    ~RenderSystem() = default;

    void Update(const std::vector<std::shared_ptr<Entity>> &entities, float deltaTime) override
    {
        glm::mat4 lightProjection, lightView;
        float near_plane = 0.1f, far_plane = 25.0f;                                        // These values might need tuning
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); // Adjust frustum to scene size
        lightView = glm::lookAt(glm::vec3(0.0f, 3.0f, -5.0f),                              // Light position (matching lightPos in default.glsl)
                                glm::vec3(0.0f, 0.0f, 0.0f),                               // Look at origin
                                glm::vec3(0.0f, 1.0f, 0.0f));                              // Up vector
        this->lightSpaceMatrix = lightProjection * lightView;                              // Update member variable

        sbo->Bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        depthShader->Use();
        depthShader->SetUniform("lightSpaceMatrix", lightSpaceMatrix);
        Draw(entities, deltaTime, *depthShader);

        // Debug GL error after depth pass
        {
            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
                std::cout << "[RenderSystem] GL error after depth pass: " << err << std::endl;
        }

        sbo->Unbind();
        fbo->Bind();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        defaultShader->Use();
        defaultShader->SetUniform("view", defaultCamera->GetView());
        defaultShader->SetUniform("projection", defaultCamera->GetProjection());
        defaultShader->SetUniform("viewPos", defaultCamera->GetPosition());
        defaultShader->SetUniform("lightColor", glm::vec3(1.0f));
        defaultShader->SetUniform("lightPos", glm::vec3(0.0f, 3.0f, -5.0f));
        defaultShader->SetUniform("objectColor", glm::vec3(1.0f));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sbo->GetDepthTexture());
        defaultShader->SetUniform("shadowMap", 1);
        defaultShader->SetUniform("lightSpaceMatrix", lightSpaceMatrix);
        Draw(entities, deltaTime, *defaultShader);

        // Debug GL error after default pass
        {
            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
                std::cout << "[RenderSystem] GL error after default pass: " << err << std::endl;
        }

        fbo->Unbind();

        // Blit multisampled FBO to intermediate FBO
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->GetID());  // Use FBO ID for read framebuffer
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ifbo->GetID()); // Use FBO ID for draw framebuffer
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, this->width, this->height, 0, 0, this->width, this->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind default framebuffer again

        screen->Draw(*bufferShader, ifbo->GetTexture());

        // Debug GL error after screen draw
        {
            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
                std::cout << "[RenderSystem] GL error after screen draw: " << err << std::endl;
        }
    }

    void OnResize(const int &w, const int &h)
    {
        this->width = w;  // Assign new width
        this->height = h; // Assign new height

        fbo->OnResize(w, h);
        ifbo->OnResize(w, h);
        defaultCamera->OnResize(w, h);
    }

private:
    int width = 0, height = 0;
    glm::mat4 lightSpaceMatrix;
    glm::mat4 lightProjection, lightView;
    std::unique_ptr<FBO> fbo = nullptr, ifbo = nullptr;
    std::unique_ptr<SBO> sbo = nullptr;
    std::unique_ptr<Quad> screen = nullptr;
    std::unique_ptr<Camera> defaultCamera = nullptr;

    std::unique_ptr<Shader> defaultShader = nullptr;
    std::unique_ptr<Shader> depthShader = nullptr;
    std::unique_ptr<Shader> bufferShader = nullptr;

    void Draw(const std::vector<std::shared_ptr<Entity>> &entities, const float &deltaTime, const Shader &shader)
    {
        for (auto &en : entities)
            DrawRecursive(en, deltaTime, shader);
    }

    void DrawRecursive(const std::shared_ptr<Entity> &en, const float &deltaTime, const Shader &shader)
    {
        // Draw this entity
        DrawEntity(en, deltaTime, shader);

        // Draw children recursively
        for (auto &child : en->children)
            DrawRecursive(child, deltaTime, shader);
    }

    void DrawEntity(const std::shared_ptr<Entity> &en, const float &deltaTime, const Shader &shader)
    {
        for (auto &c : en->GetComponents<MeshRenderer>())
        {
            c->Render(shader);
        }
    }
};