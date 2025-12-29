#pragma once
#include <engine/components/physics/collision.hpp>

class BoxCollider : public Collider
{
public:
    glm::vec3 halfSize{1.0f};
    glm::vec3 center{0.0f};

public:
    BoxCollider()
    {
        type = CollisionType::AABB;
    }
};