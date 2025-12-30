#pragma once
#include <engine/ecs/physics.component.hpp>
#include <Bullet3/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <engine/systems/physics.hpp>

class BoxCollider3D : public Component
{
public:
    glm::vec3 halfSize{1.0f};

    btCollisionShape* CreateShape() const
    {
        return new btBoxShape(btVector3(halfSize.x, halfSize.y, halfSize.z));
    }
};
