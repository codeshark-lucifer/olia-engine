#include <engine/rendersystem.hpp>
#include <functional>
#include <engine/utils/entity.hpp> // Include for Entity definition
#include <engine/utils/component.hpp> // Include for Component definition
#include <engine/meshrenderer.hpp> // Include for MeshRenderer

void RenderSystem::Update(std::vector<std::shared_ptr<Entity>>& entities, float dt)
{
    // This Update method could be used for culling, setting up render passes, etc.
    // For now, it will simply call RenderScene.
    // However, the main rendering call will likely be handled directly by Scene,
    // which will pass the appropriate shader.
    // So, this Update method might remain empty or be used for more generic render system updates.
}

void RenderSystem::RenderScene(std::vector<std::shared_ptr<Entity>>& entities, Shader& shader)
{
    // Function to recursively render entity and its children
    std::function<void(const std::shared_ptr<Entity>&)> renderEntityAndChildren = 
        [&](const std::shared_ptr<Entity>& currentEntity) {
        auto meshRenderers = currentEntity->GetComponents<MeshRenderer>();
        for (auto& meshRenderer : meshRenderers)
        {
            meshRenderer->Render(shader);
        }

        for (auto& child : currentEntity->children)
        {
            renderEntityAndChildren(child);
        }
    };

    for (auto& entity : entities)
    {
        renderEntityAndChildren(entity);
    }
}
