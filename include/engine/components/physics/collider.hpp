#pragma once
#include <engine/ecs/entity.hpp>

enum class ColliderType
{
    Cube,
    Sphere
};

class Collider : public Component
{
public:
    bool isTrigger = false;
    ColliderType type = ColliderType::Cube;
    Collider() {}
};