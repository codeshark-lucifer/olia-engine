#pragma once
#include <engine/ecs/physics.component.hpp>
#include <Bullet3/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <engine/systems/physics.hpp>

enum class CollisionType
{
    BOX,
    SPHERE,
    CAPSULE,
    MESH
};

class Collider : public Component
{
public:
    CollisionType type = CollisionType::BOX;
    virtual btCollisionShape *CreateShape() const { return nullptr; }
};
