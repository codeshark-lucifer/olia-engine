#pragma once
#include <engine/components/physics/collider.hpp>

class SphereCollider : public Collider
{
public:
    float radius = 1.0f;
    glm::vec3 center = glm::vec3(0.0f);

    SphereCollider()
    {
        type = ColliderType::Sphere;
    }
};