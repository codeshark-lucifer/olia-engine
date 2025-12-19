#pragma once
#include <engine/ec/entity.hpp>

class Collider : public Component
{
public:
    virtual ~Collider() = default;
    virtual bool CheckCollision(Collider* other) = 0;
};