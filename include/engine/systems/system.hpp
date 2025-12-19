#pragma once
#include <engine/ec/entity.hpp>

class System
{
public:
    ~System() = default;
    virtual void Update(const std::vector<std::shared_ptr<Entity>>& entities, float dt) {}
};