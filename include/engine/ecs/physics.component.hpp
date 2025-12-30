#pragma once
#include <engine/ecs/component.hpp>
#include <Bullet3/btBulletDynamicsCommon.h>

class PhysicsComponent : public Component
{
public:
    btRigidBody* body = nullptr;
    btCollisionShape* shape = nullptr;
};
