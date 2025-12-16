#pragma once
#include <engine/system.hpp>
#include <engine/utils/entity.hpp> // Required for Entity and Component definitions

class UpdateSystem : public System
{
public:
    UpdateSystem() : System("UpdateSystem") {}
    ~UpdateSystem() override = default;

    void Update(std::vector<std::shared_ptr<Entity>>& entities, float dt) override;
};
