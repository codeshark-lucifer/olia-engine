#include "engine/scene.hpp"
#include <glad/glad.h>
#include <engine/utils/config.h>
#include <glm/gtc/matrix_transform.hpp>

Scene::Scene(const std::string &n, int msaaSamples) : name(n), msaaSamples(msaaSamples), width(SCREEN_WIDTH), height(SCREEN_HEIGHT)
{
    // std::cout << "Scene Created!\n";
    defaultShader = std::make_shared<Shader>("assets/shaders/glsl/default.glsl");
    defaultCamera = std::make_shared<Camera>(SCREEN_WIDTH, SCREEN_HEIGHT);

    defaultFrameBufferShader = std::make_shared<Shader>("assets/shaders/vert/render.vert", "assets/shaders/frag/fbo.frag");
    defaultFrameBuffer = std::make_shared<FBO>(SCREEN_WIDTH, SCREEN_HEIGHT, msaaSamples);
    intermediateFrameBuffer = std::make_shared<FBO>(SCREEN_WIDTH, SCREEN_HEIGHT, 1); // Non-multisampled
    screen = std::make_shared<Quad>();

    defaultShadowBuffer = std::make_shared<ShadowBuffer>(SHADOW_WIDTH, SHADOW_HEIGHT);
    defaultShadowShader = std::make_shared<Shader>("assets/shaders/glsl/depth.glsl");

    updateSystem = std::make_shared<UpdateSystem>();
    renderSystem = std::make_shared<RenderSystem>();
}

void Scene::Begin() const
{
    // std::cout << "Scene Awake\n";
    for (auto &en : entities)
    {
        en->Begin();
    }
}

void Scene::Update(float dt)
{
    // 1. directional Light space matrix calculation
    glm::mat4 lightProjection, lightView;
    float near_plane = 0.1f, far_plane = 25.0f;                                        // These values might need tuning
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); // Adjust frustum to scene size
    lightView = glm::lookAt(glm::vec3(0.0f, 3.0f, -5.0f),                              // Light position (matching lightPos in default.glsl)
                            glm::vec3(0.0f, 0.0f, 0.0f),                               // Look at origin
                            glm::vec3(0.0f, 1.0f, 0.0f));                              // Up vector
    this->lightSpaceMatrix = lightProjection * lightView;                              // Update member variable

    defaultShadowBuffer->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    defaultShadowShader->Use();
    defaultShadowShader->SetUniform("lightSpaceMatrix", lightSpaceMatrix); // light projection*view

    renderSystem->RenderScene(entities, *defaultShadowShader); // Render scene for shadow pass

    defaultShadowBuffer->Unbind();

    defaultFrameBuffer->Bind();
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
    glBindTexture(GL_TEXTURE_2D, defaultShadowBuffer->GetDepthTexture());
    defaultShader->SetUniform("shadowMap", 1);
    defaultShader->SetUniform("lightSpaceMatrix", lightSpaceMatrix);

    updateSystem->Update(entities, dt);                  // Update entities using UpdateSystem
    renderSystem->RenderScene(entities, *defaultShader); // Render scene for main pass

    defaultFrameBuffer->Unbind();

    // Blit multisampled FBO to intermediate FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFrameBuffer->GetTexture());      // GetTexture returns ID for multisampled FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFrameBuffer->GetTexture()); // GetTexture returns ID for regular FBO
    glBlitFramebuffer(0, 0, this->width, this->height, 0, 0, this->width, this->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind default framebuffer again

    screen->Draw(*defaultFrameBufferShader, intermediateFrameBuffer->GetTexture());
}

void Scene::AddEntity(std::shared_ptr<Entity> en)
{
    entities.push_back(std::move(en));
}

std::shared_ptr<Entity> Scene::CreateEntity(const std::string &n)
{
    auto en = Entity::Create(n);
    AddEntity(en);
    return en;
}

void Scene::OnResize(int w, int h)
{
    this->width = w;  // Assign new width
    this->height = h; // Assign new height
    defaultCamera->OnResize(w, h);
    defaultFrameBuffer->OnResize(w, h);
    intermediateFrameBuffer->OnResize(w, h);
}
