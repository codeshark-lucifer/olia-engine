#pragma once
#include <engine/ecs/entity.hpp>

enum class CollisionType
{
    AABB,
    Sphere
};

class Collider : public Component
{
public:
    CollisionType type = CollisionType::AABB;
    bool isTrigger;

public:
    Collider() {}
};