#pragma once
#include <engine/ecs/system.hpp>
#include <engine/components/meshrenderer.hpp>
#include <engine/components/camera.hpp>

#include <engine/buffers/fbo.hpp>
#include <engine/buffers/sbo.hpp>
#include <engine/buffers/quad.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/config.h>

class RenderSystem : public System
{
public:
    RenderSystem(int w, int h)
        : System("RenderSystem"), width(w), height(h)
    {
        // Load shaders
        sceneShader = std::make_shared<Shader>("assets/shaders/scene.glsl");
        depthShader = std::make_shared<Shader>("assets/shaders/depth.glsl");
        screenShader = std::make_shared<Shader>("assets/shaders/screen.glsl");

        // Create buffers
        msaaFBO = std::make_unique<FBO>(width, height, 4); // MSAA x4
        resolvedFBO = std::make_unique<FBO>(width, height);
        sbo = std::make_unique<SBO>(SHADOW_SIZE);
        quad = std::make_unique<Quad>();

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // default background color
    }

    void Update(const std::vector<std::shared_ptr<Entity>> &entities, const float &)
    {
        UpdateCamera(entities);
        ShadowPass(entities);
        ScenePass(entities);
        ResolveMSAA();
        ScreenPass();
    }

    void OnResize(const int &w, const int &h) override
    {
        width = w;
        height = h;
        msaaFBO->Resize(width, height);
        resolvedFBO->Resize(width, height);
        glViewport(0, 0, width, height);
    }

private:
    int width = 0;
    int height = 0;

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    glm::vec3 lightDir = glm::normalize(glm::vec3(-2.0f, -4.0f, -1.0f));

    std::shared_ptr<Shader> sceneShader;
    std::shared_ptr<Shader> depthShader;
    std::shared_ptr<Shader> screenShader;

    std::unique_ptr<FBO> msaaFBO;
    std::unique_ptr<FBO> resolvedFBO;
    std::unique_ptr<SBO> sbo;
    std::unique_ptr<Quad> quad;

private:
    // -------------------------------
    // CAMERA
    // -------------------------------
    void UpdateCamera(const std::vector<std::shared_ptr<Entity>> &entities)
    {
        for (auto &en : entities)
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

    // -------------------------------
    // SHADOW PASS
    // -------------------------------
    void ShadowPass(const std::vector<std::shared_ptr<Entity>> &entities)
    {
        glCullFace(GL_FRONT); // prevent shadow acne
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        sbo->Bind();

        glm::mat4 lightProj = glm::ortho(-100.f, 100.f, -100.f, 100.f, 0.01f, 200.f);
        glm::mat4 lightView = glm::lookAt(-lightDir * 20.0f, glm::vec3(0.0f), glm::vec3(0, 1, 0));
        glm::mat4 lightSpace = lightProj * lightView;

        depthShader->Use();
        depthShader->SetUniform("lightSpace", lightSpace);

        for (auto &en : entities)
            DrawEntity(en, *depthShader, true);

        SBO::Unbind(width, height);
        glCullFace(GL_BACK);

        // Bind shadow map to scene shader
        sceneShader->Use();
        sceneShader->SetUniform("lightSpace", lightSpace);
        sceneShader->SetUniform("shadowMap", 5);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, sbo->GetDepthMap());
        glDisable(GL_DEPTH_TEST); // Disable depth test after depth map generation
    }

    // -------------------------------
    // SCENE PASS (MSAA)
    // -------------------------------
    void ScenePass(const std::vector<std::shared_ptr<Entity>> &entities)
    {
        msaaFBO->Bind();

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sceneShader->Use();
        sceneShader->SetUniform("view", view);
        sceneShader->SetUniform("projection", projection);
        sceneShader->SetUniform("lightDir", lightDir);

        for (auto &en : entities)
            DrawEntity(en, *sceneShader, false);
    }

    // -------------------------------
    // MSAA RESOLVE
    // -------------------------------
    void ResolveMSAA()
    {
        if (msaaFBO->IsMSAA())
            msaaFBO->BlitTo(*resolvedFBO);
    }

    // -------------------------------
    // SCREEN PASS
    // -------------------------------
    void ScreenPass()
    {
        FBO::Unbind(width, height);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader->Use();
        screenShader->SetUniform("screenTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, resolvedFBO->GetColorTexture());

        quad->Draw();
    }

    // -------------------------------
    // DRAW ENTITY RECURSIVE
    // -------------------------------
    void DrawEntity(const std::shared_ptr<Entity> &en, const Shader &shader, bool isDepthPass)
    {
        if (en->HasComponent<MeshRenderer>())
        {
            if (isDepthPass)
            {
                depthShader->Use();                             // Ensure depth shader is active
                depthShader->SetUniform("model", en->matrix()); // Set model matrix for depth shader
                en->GetComponent<MeshFilter>()->mesh->DrawDepth();
            }
            else
            {
                en->GetComponent<MeshRenderer>()->Render(shader);
            }
        }

        for (auto &child : en->children)
            DrawEntity(child, shader, isDepthPass);
    }
};
