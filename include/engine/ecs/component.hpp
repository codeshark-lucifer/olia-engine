#pragma once
#include <memory>
class Entity;
class Component
{
public:
    bool enabled = true;
    std::weak_ptr<Entity> entity;
    virtual ~Component() = default;
    virtual void Begin() {}
    virtual void Update(const float &deltaTime) {}
};