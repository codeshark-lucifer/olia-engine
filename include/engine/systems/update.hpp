#pragma once
#include <engine/systems/system.hpp>
#include <functional>

class UpdateSystem : public System
{
public:
    UpdateSystem() {}
    void Update(const std::vector<std::shared_ptr<Entity>> &entities, float dt)
    {
        // Recursive lambda using std::function
        std::function<void(const std::shared_ptr<Entity> &)> updateEntity;

        updateEntity = [&](const std::shared_ptr<Entity> &en)
        {
            // Update this entity's components
            for (auto &comp : en->components)
            {
                comp->FixedUpdate(dt);
                comp->Update(dt);
                comp->LateUpdate(dt);
            }

            // Update children
            for (auto &child : en->children)
            {
                updateEntity(child);
            }
        };

        // Start from root entities
        for (auto &en : entities)
        {
            updateEntity(en);
        }
    }
};