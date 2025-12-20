#pragma once
#include <engine/systems/system.hpp>

class UpdateSystem : public System
{
public:
    UpdateSystem() {}
    void Update(const std::vector<std::shared_ptr<Entity>> &entities, float deltaTime)
    {
        // Start from root entities
        for (auto &en : entities)
        {
            UpdateEntity(en, deltaTime);
        }
    }

private:
    void UpdateEntity(const std::shared_ptr<Entity> &en, const float &deltaTime)
    {
        for (auto &comp : en->components)
        {
            comp->FixedUpdate(deltaTime);
            comp->Update(deltaTime);
            comp->LateUpdate(deltaTime);
        }

        for (auto &child : en->children)
        {
            UpdateEntity(child, deltaTime);
        }
    }
};