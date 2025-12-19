#include <engine/scene.hpp>
#include <engine/config.h>

#include <engine/systems/update.hpp>
#include <engine/systems/physics.hpp>

Scene::Scene(const int &w, const int &h, const std::string &n) : width(w), height(h), name(n)
{
    defaultCamera = std::make_unique<Camera>(w, h);

    defaultShader = std::make_unique<Shader>("assets/shaders/glsl/default.glsl");
    depthShader = std::make_unique<Shader>("assets/shaders/glsl/depth.glsl");
    frameShader = std::make_unique<Shader>("assets/shaders/vert/render.vert", "assets/shaders/frag/fbo.frag");

    sbo = std::make_unique<SBO>(SHADOW_WIDHT, SHADOW_HEIGHT);
    fbo = std::make_unique<FBO>(w, h, MSAASAMPLES);
    ifbo = std::make_unique<FBO>(w, h, 1);
    screen = std::make_unique<Quad>();

    render = std::make_unique<RenderSystem>();
    systems.push_back(std::make_unique<UpdateSystem>());
    systems.push_back(std::make_unique<PhysicsSystem>());
}

void Scene::Begin()
{
        // Recursive lambda using std::function
        std::function<void(const std::shared_ptr<Entity> &)> beginEntity;

        beginEntity = [&](const std::shared_ptr<Entity> &en)
        {
            // Update this entity's components
            for (auto &comp : en->components)
            {
                comp->Awake();
                comp->Start();
            }

            // Update children
            for (auto &child : en->children)
            {
                beginEntity(child);
            }
        };

        // Start from root entities
        for (auto &en : entities)
        {
            beginEntity(en);
        }
}

void Scene::Update(float dt)
{
    for(auto& s : systems) {
        s->Update(entities, dt);
    }
    glm::mat4 lightProjection, lightView;
    float near_plane = 0.1f, far_plane = 25.0f;                                        // These values might need tuning
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); // Adjust frustum to scene size
    lightView = glm::lookAt(glm::vec3(0.0f, 3.0f, -5.0f),                              // Light position (matching lightPos in default.glsl)
                            glm::vec3(0.0f, 0.0f, 0.0f),                               // Look at origin
                            glm::vec3(0.0f, 1.0f, 0.0f));                              // Up vector
    this->lightSpaceMatrix = lightProjection * lightView;

    sbo->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    depthShader->Use();
    depthShader->SetUniform("lightSpaceMatrix", lightSpaceMatrix);

    render->Render(entities, *depthShader);

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

    render->Render(entities, *defaultShader);

    fbo->Unbind();

    // Blit multisampled FBO to intermediate FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->GetTexture());  // GetTexture returns ID for multisampled FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ifbo->GetTexture()); // GetTexture returns ID for regular FBO
    glBlitFramebuffer(0, 0, this->width, this->height, 0, 0, this->width, this->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind default framebuffer again
    screen->Draw(*frameShader, ifbo->GetTexture());
}

void Scene::OnResize(const int &w, const int &h)
{
    this->width = w;
    this->height = h;

    defaultCamera->OnResize(w, h);
    fbo->OnResize(w, h);
    ifbo->OnResize(w, h);
}

void Scene::AddEntity(const std::shared_ptr<Entity> &en)
{
    entities.push_back(en);
}