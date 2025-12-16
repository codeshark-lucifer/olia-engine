#pragma once
#include <engine/system.hpp>
#include <engine/utils/entity.hpp> // Required for Entity and Component definitions
#include <engine/meshrenderer.hpp> // Required for MeshRenderer
#include <engine/shader.hpp> // Required for Shader

class RenderSystem : public System
{
public:
    RenderSystem() : System("RenderSystem") {}
    ~RenderSystem() override = default;

    // The RenderSystem's update will perform the rendering
    // It takes the main shader to be used for rendering the scene
    void Update(std::vector<std::shared_ptr<Entity>>& entities, float dt) override;
    void RenderScene(std::vector<std::shared_ptr<Entity>>& entities, Shader& shader);
};
