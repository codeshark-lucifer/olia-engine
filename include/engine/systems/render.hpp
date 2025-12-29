#pragma once
#include <engine/ecs/system.hpp>
#include <engine/shader.hpp>
#include <engine/components/meshrenderer.hpp>
#include <engine/components/camera.hpp>
#include <engine/components/light.hpp>

#include <engine/buffers/quad.hpp>
#include <engine/buffers/fbo.hpp>
#include <engine/buffers/sbo.hpp>

#include <engine/input.hpp>

class RenderSystem : public System
{
public:
    RenderSystem(int width, int height) : System("RenderSystem"), width(width), height(height)
    {
        defaultShader = std::make_shared<Shader>("assets/shaders/scene.glsl");
        bufferShader = std::make_shared<Shader>("assets/shaders/screen.glsl");
        depthShader = std::make_shared<Shader>("assets/shaders/depth.glsl");

        fbo = std::make_shared<FBO>(width, height, 8);
        ifbo = std::make_shared<FBO>(width, height, 1);
        sbo = std::make_shared<SBO>(2048);

        screen = std::make_shared<Quad>();

        bufferShader->Use();
        bufferShader->SetUniform("screenTexture", 0);

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
    }

    void Update(std::vector<std::shared_ptr<Entity>> &entities, float /*deltaTime*/)
    {
        frameLights.clear();
        frameLights.reserve(32);

        for (auto &entity : entities)
            CollectLights(entity, frameLights);
    }

    void Render(std::vector<std::shared_ptr<Entity>> &entities) override
    {
        if (frameLights.empty())
            return;
        auto mainLight = frameLights[0];
        if (mainLight->type != LightType::Directional)
            return;

        // 1. Light view-projection
        glm::vec3 lightDir = glm::normalize(mainLight->direction);
        glm::vec3 lightPos = -lightDir * 10.0f;
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
        glm::mat4 lightProj = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 50.f);
        glm::mat4 lightVP = lightProj * lightView;

        RenderShadowMap(entities, lightVP);
        RenderScene(entities, lightVP);
        RenderFinalQuad();
    }

    void OnResize(int w, int h, std::vector<std::shared_ptr<Entity>> &entities) override
    {
        width = w;
        height = h;

        for (auto &entity : entities)
            if (auto camera = entity->GetComponent<Camera>())
                camera->OnResize(w, h);
    }

private:
    float gamma = 1.1f;
    int width, height;

    std::shared_ptr<Shader> defaultShader;
    std::shared_ptr<Shader> bufferShader;
    std::shared_ptr<Shader> depthShader;

    std::shared_ptr<FBO> fbo;
    std::shared_ptr<FBO> ifbo;
    std::shared_ptr<SBO> sbo;
    std::shared_ptr<Quad> screen;

    std::vector<std::shared_ptr<Light>> frameLights;

    // ------------------------
    // LIGHT COLLECTION
    // ------------------------
    void CollectLights(const std::shared_ptr<Entity> &entity, std::vector<std::shared_ptr<Light>> &lights)
    {
        if (auto light = entity->GetComponent<Light>())
            lights.push_back(light);

        for (auto &child : entity->GetChildren())
            CollectLights(child, lights);
    }

    // ------------------------
    // SHADOW PASS
    // ------------------------
    void RenderShadowMap(const std::vector<std::shared_ptr<Entity>> &entities, const glm::mat4 &lightVP)
    {
        sbo->Bind();
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glClear(GL_DEPTH_BUFFER_BIT);

        depthShader->Use();
        depthShader->SetUniform("lightViewProjection", lightVP);

        DrawEntities(entities, *depthShader);

        sbo->Unbind();
    }

    // ------------------------
    // SCENE PASS
    // ------------------------
    void RenderScene(const std::vector<std::shared_ptr<Entity>> &entities, const glm::mat4 &lightVP)
    {
        fbo->Bind();
        ClearBuffer();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        defaultShader->Use();
        defaultShader->SetUniform("lightViewProjection", lightVP);

        int lightIndex = 0;
        for (auto &light : frameLights)
        {
            std::string prefix = "lights[" + std::to_string(lightIndex) + "]";
            defaultShader->SetUniform(prefix + ".type", (int)light->type);
            defaultShader->SetUniform(prefix + ".direction", light->direction);
            defaultShader->SetUniform(prefix + ".position", light->entity.lock()->transform.position);
            defaultShader->SetUniform(prefix + ".color", light->color);
            defaultShader->SetUniform(prefix + ".intensity", light->intensity);
            defaultShader->SetUniform(prefix + ".range", light->range);
            lightIndex++;
        }
        defaultShader->SetUniform("lightCount", lightIndex);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sbo->GetDepthMap());
        defaultShader->SetUniform("shadowMap", 1);

        // Update camera matrices
        for (auto &entity : entities)
            if (auto camera = entity->GetComponent<Camera>())
                camera->SetUniform(*defaultShader);

        DrawEntities(entities, *defaultShader);

        fbo->BlitTo(*ifbo);
        FBO::Unbind(width, height);
    }

    // ------------------------
    // FINAL QUAD PASS
    // ------------------------
    void RenderFinalQuad()
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT);

        bufferShader->Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ifbo->GetColorTexture());
        screen->Draw();
    }

    void ClearBuffer()
    {
        float val = pow(0.1f, gamma);
        glClearColor(val, val, val, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void DrawEntities(const std::vector<std::shared_ptr<Entity>> &entities, Shader &shader)
    {
        static std::vector<std::shared_ptr<MeshRenderer>> renderers;
        renderers.clear();
        renderers.reserve(entities.size() * 2);

        for (auto &entity : entities)
            CollectRenderers(entity, renderers);

        for (auto &render : renderers)
            render->Render(shader);
    }

    void CollectRenderers(const std::shared_ptr<Entity> &entity, std::vector<std::shared_ptr<MeshRenderer>> &outRenderers)
    {
        if (auto render = entity->GetComponent<MeshRenderer>())
            outRenderers.push_back(render);

        for (auto &child : entity->GetChildren())
            CollectRenderers(child, outRenderers);
    }
};
