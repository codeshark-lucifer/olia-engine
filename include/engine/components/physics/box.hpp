#pragma once
#include <engine/components/physics/collider.hpp>

class BoxCollider : public Collider
{
public:
    glm::vec3 size = glm::vec3(1.0f);
    glm::vec3 center = glm::vec3(0.0f);
    BoxCollider()
    {
        type = ColliderType::Cube;
    }
};