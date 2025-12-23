#pragma once
#include <engine/ecs/system.hpp>
#include <engine/components/meshrenderer.hpp>
#include <engine/components/camera.hpp>

#include <engine/buffers/fbo.hpp>
#include <engine/buffers/ifbo.hpp>
#include <engine/buffers/sbo.hpp>
#include <engine/buffers/quad.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class RenderSystem : public System
{
public:
    RenderSystem(int w, int h)
        : System("RenderSystem"), width(w), height(h)
    {
        // Shaders
        sceneShader  = std::make_shared<Shader>("assets/shaders/scene.glsl");
        depthShader  = std::make_shared<Shader>("assets/shaders/depth.glsl");
        screenShader = std::make_shared<Shader>("assets/shaders/screen.glsl");

        // Buffers
        ifbo   = std::make_unique<IFBO>(width, height, 4); // MSAA x4
        fbo    = std::make_unique<FBO>(width, height);
        sbo    = std::make_unique<SBO>(2048);
        quad   = std::make_unique<Quad>();

        glEnable(GL_DEPTH_TEST);
    }

    // -------------------------------
    // UPDATE
    // -------------------------------
    void Update(const std::vector<std::shared_ptr<Entity>>& entities, const float&) override
    {
        UpdateCamera(entities);

        ShadowPass(entities);
        ScenePass(entities);
        ResolveMSAA();
        ScreenPass();
    }

    void OnResize(int w, int h)
    {
        width = w;
        height = h;

        ifbo = std::make_unique<IFBO>(width, height, 4);
        fbo  = std::make_unique<FBO>(width, height);
    }

private:
    // -------------------------------
    // STATE
    // -------------------------------
    int width = 0;
    int height = 0;

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // -------------------------------
    // SHADERS
    // -------------------------------
    std::shared_ptr<Shader> sceneShader;
    std::shared_ptr<Shader> depthShader;
    std::shared_ptr<Shader> screenShader;

    // -------------------------------
    // BUFFERS
    // -------------------------------
    std::unique_ptr<IFBO> ifbo;   // MSAA
    std::unique_ptr<FBO>  fbo;    // resolved
    std::unique_ptr<SBO>  sbo;    // shadow
    std::unique_ptr<Quad> quad;

private:
    // ===============================
    // CAMERA
    // ===============================
    void UpdateCamera(const std::vector<std::shared_ptr<Entity>>& entities)
    {
        for (auto& en : entities)
        {
            if (!en->HasComponent<Camera>())
                continue;

            auto cam = en->GetComponent<Camera>();
            cam->OnResize(width, height);

            view = cam->GetView();
            projection = cam->GetProjection();
            return;
        }
    }

    // ===============================
    // SHADOW PASS
    // ===============================
    void ShadowPass(const std::vector<std::shared_ptr<Entity>>& entities)
    {
        sbo->Bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        glm::vec3 lightDir = glm::normalize(glm::vec3(-2.0f, -4.0f, -1.0f));

        glm::mat4 lightProj =
            glm::ortho(-20.f, 20.f, -20.f, 20.f, 1.f, 50.f);

        glm::mat4 lightView =
            glm::lookAt(-lightDir * 20.0f, glm::vec3(0.0f), glm::vec3(0, 1, 0));

        glm::mat4 lightSpace = lightProj * lightView;

        depthShader->Use();
        depthShader->SetUniform("lightSpace", lightSpace);

        for (auto& en : entities)
            DrawEntity(en, *depthShader);

        SBO::Unbind(width, height);

        sceneShader->Use();
        sceneShader->SetUniform("lightSpace", lightSpace);
        sceneShader->SetUniform("shadowMap", 5);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, sbo->GetDepthMap());
    }

    // ===============================
    // SCENE PASS (MSAA)
    // ===============================
    void ScenePass(const std::vector<std::shared_ptr<Entity>>& entities)
    {
        ifbo->Bind();

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sceneShader->Use();
        sceneShader->SetUniform("view", view);
        sceneShader->SetUniform("projection", projection);
        sceneShader->SetUniform("lightDir", glm::normalize(glm::vec3(-2, -4, -1)));
        sceneShader->SetUniform("viewPos", glm::vec3(0, 0, 5));

        for (auto& en : entities)
            DrawEntity(en, *sceneShader);

        IFBO::Unbind();
    }

    // ===============================
    // MSAA RESOLVE
    // ===============================
    void ResolveMSAA()
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, ifbo->GetID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->GetID());

        glBlitFramebuffer(
            0, 0, width, height,
            0, 0, width, height,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // ===============================
    // SCREEN PASS
    // ===============================
    void ScreenPass()
    {
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader->Use();
        screenShader->SetUniform("screenTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbo->GetTexture());

        quad->Draw();
    }

    // ===============================
    // DRAW ENTITY (RECURSIVE)
    // ===============================
    void DrawEntity(const std::shared_ptr<Entity>& en, const Shader& shader)
    {
        if (en->HasComponent<MeshRenderer>())
            en->GetComponent<MeshRenderer>()->Render(shader);

        for (auto& child : en->children)
            DrawEntity(child, shader);
    }
};
