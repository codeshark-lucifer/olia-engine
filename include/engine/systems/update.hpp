#pragma once
#include <engine/ecs/system.hpp>

class UpdateSystem : public System
{
public:
    UpdateSystem() : System("UpdateSystem") {}

    void OnAttach(std::vector<std::shared_ptr<Entity>>& entities) override
    {
        for (auto& entity : entities)
            TraverseAndAttach(entity);
    }

    void Update(std::vector<std::shared_ptr<Entity>>& entities, float deltaTime) override
    {
        for (auto& entity : entities)
            TraverseAndUpdate(entity, deltaTime);
    }

private:
    void TraverseAndAttach(std::shared_ptr<Entity> entity)
    {
        for (auto& comp : entity->GetAllComponents())
            comp->OnAttach();

        for (auto& child : entity->GetChildren())
            TraverseAndAttach(child);
    }

    void TraverseAndUpdate(std::shared_ptr<Entity> entity, float deltaTime)
    {
        for (auto& comp : entity->GetAllComponents())
            comp->Update(deltaTime);

        for (auto& child : entity->GetChildren())
            TraverseAndUpdate(child, deltaTime);
    }
};
