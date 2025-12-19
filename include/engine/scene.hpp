#pragma once
#include <engine/systems/render.hpp>
#include <engine/systems/system.hpp>
#include <engine/components/camera.hpp>

#include <engine/ec/entity.hpp>
#include <engine/shader.hpp>
#include <engine/buffers/fbo.hpp>
#include <engine/buffers/quad.hpp>
#include <engine/buffers/sbo.hpp>

class Scene
{
public:
    Scene(const int &w, const int &h, const std::string &n);
    void Begin();
    void Update(float dt);
    void OnResize(const int &w, const int &h);
    void AddEntity(const std::shared_ptr<Entity>& en);
    const std::vector<std::shared_ptr<Entity>>& GetEntities() const { return entities; }

private:
    int width = 0,
        height = 0;
    glm::mat4 lightSpaceMatrix = glm::mat4(0.0f);

    std::string name = "SampleScene";
    std::vector<std::shared_ptr<Entity>> entities;
    std::vector<std::unique_ptr<System>> systems;

    std::unique_ptr<Camera> defaultCamera = nullptr;

    std::unique_ptr<Shader> defaultShader = nullptr;
    std::unique_ptr<Shader> depthShader = nullptr;
    std::unique_ptr<Shader> frameShader = nullptr;

    std::unique_ptr<SBO> sbo = nullptr;
    std::unique_ptr<FBO> fbo = nullptr;
    std::unique_ptr<FBO> ifbo = nullptr;
    std::unique_ptr<Quad> screen = nullptr;

    std::unique_ptr<RenderSystem> render = nullptr;
};