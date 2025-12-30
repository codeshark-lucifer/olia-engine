#pragma once
#include <engine/components/physics/collider/collider.hpp>

class BoxCollider3D : public Collider
{
public:
    glm::vec3 halfSize{1.0f};

public:
    BoxCollider3D() { type = CollisionType::BOX; }

    btCollisionShape *CreateShape() const override
    {
        return new btBoxShape(btVector3(halfSize.x, halfSize.y, halfSize.z));
    }
};
