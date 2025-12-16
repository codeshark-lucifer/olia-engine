#pragma once
#include <engine/component.hpp>
#include <engine/transform.hpp>
#include <engine/entity.hpp> // For accessing entity's transform

class Rotator : public Component
{
public:
    Rotator() = default;
    ~Rotator() = default;

    void Update(float dt) override;
};