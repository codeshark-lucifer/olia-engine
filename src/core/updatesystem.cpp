#include <engine/updatesystem.hpp>
#include <functional>
#include <engine/utils/entity.hpp> // Include for Entity definition
#include <engine/utils/component.hpp> // Include for Component definition

void UpdateSystem::Update(std::vector<std::shared_ptr<Entity>>& entities, float dt)
{
    for (auto& entity : entities)
    {
        // Recursively update children
        std::function<void(const std::shared_ptr<Entity>&)> updateEntityAndChildren = 
            [&](const std::shared_ptr<Entity>& currentEntity) {
            for (auto& component : currentEntity->components)
            {
                component->FixedUpdate(dt);
                component->Update(dt);
                component->LateUpdate(dt);
            }
            for (auto& child : currentEntity->children)
            {
                updateEntityAndChildren(child);
            }
        };
        updateEntityAndChildren(entity);
    }
}
