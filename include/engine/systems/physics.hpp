#pragma once
#include <engine/systems/system.hpp>
#include <iostream>

class PhysicsSystem : public System
{
public:
    PhysicsSystem() {}
    void Update(const std::vector<std::shared_ptr<Entity>> &entities, float dt)
    {
        
    }
};