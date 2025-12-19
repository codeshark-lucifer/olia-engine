#pragma once
#include <engine/systems/system.hpp>
#include <engine/components/physics/collider.hpp>
#include <functional>
#include <iostream>

class PhysicsSystem : public System
{
public:
    PhysicsSystem() {}
    void Update(const std::vector<std::shared_ptr<Entity>> &entities, float dt)
    {
        // Recursive lambda using std::function
        std::function<void(const std::shared_ptr<Entity> &)> updateEntity;

        updateEntity = [&](const std::shared_ptr<Entity> &en)
        {
            // Update this entity's components
            for (auto &colA : en->GetComponents<Collider>())
            {
                for (auto &colB : en->GetComponents<Collider>())
                {
                    // check if colA && colB collide
                    // std::cout << "ChecK Collided?\n";
                }
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