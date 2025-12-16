#pragma once
#include <engine/utils/component.hpp>
#include <engine/utils/transform.hpp>
#include <engine/utils/entity.hpp> // For accessing entity's transform

class Rotator : public Component
{
public:
    Rotator() = default;
    ~Rotator() = default;

    void Update(float dt);
};