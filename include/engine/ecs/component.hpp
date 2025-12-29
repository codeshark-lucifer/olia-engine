#pragma once
#include <memory>

class Entity;
class Component
{
public:
    std::weak_ptr<Entity> entity;

public:
    virtual ~Component() = default;
    virtual void OnAttach() {}
    virtual void Update(float deltaTime) {}
};