#pragma once
#include <engine/components/physics/collider/collider.hpp>

class CapsuleCollider3D : public Collider
{
public:
    float radius{1.0f};
    float height{2.0f};

public:
    CapsuleCollider3D() { type = CollisionType::CAPSULE; }

    btCollisionShape *CreateShape() const override
    {
        return new btCapsuleShape(btScalar(radius), btScalar(height));
    }
};
