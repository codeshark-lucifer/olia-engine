#pragma once
#include <engine/components/physics/collision.hpp>

class SphereCollider : public Collider
{
public:
    float radius = 1.0f;
    glm::vec3 center{0.0f};

public:
    SphereCollider()
    {
        type = CollisionType::Sphere;
    }
};