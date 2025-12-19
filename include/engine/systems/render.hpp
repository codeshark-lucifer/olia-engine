#pragma once
#include <engine/ec/entity.hpp>
#include <engine/components/meshrenderer.hpp>
#include <functional>

class Shader;
class Entity;

class RenderSystem
{
public:
    ~RenderSystem() = default;
    void Render(const std::vector<std::shared_ptr<Entity>> &entities, const Shader &shader)
    {
        std::function<void(const std::shared_ptr<Entity> &)> renderEntityAndChildren =
            [&](const std::shared_ptr<Entity> &currentEntity)
        {
            auto meshRenderers = currentEntity->GetComponents<MeshRenderer>();
            for (auto &meshRenderer : meshRenderers)
            {
                meshRenderer->Render(shader);
            }

            for (auto &child : currentEntity->children)
            {
                renderEntityAndChildren(child);
            }
        };

        for (auto &entity : entities)
        {
            renderEntityAndChildren(entity);
        }
    }
}

;