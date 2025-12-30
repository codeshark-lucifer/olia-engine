#pragma once
#include <engine/components/physics/collider/collider.hpp>

class SphereCollider3D : public Collider
{
public:
    float radius{1.0f};

public:
    SphereCollider3D() { type = CollisionType::SPHERE; }

    btCollisionShape *CreateShape() const override
    {
        return new btSphereShape(btScalar(radius));
    }
};
