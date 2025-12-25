#pragma once
#include <string>
#include <vector>
#include <engine/ecs/entity.hpp>

class System
{
public:
    std::string name = "System";

public:
    System(const std::string &n) : name(n) {}
    virtual ~System() = default;
    
    virtual void Begin(const std::vector<std::shared_ptr<Entity>> &entities) {}
    virtual void Update(const std::vector<std::shared_ptr<Entity>> &entities, const float &deltaTime) {}
    virtual void OnResize(const int &width, const int &height) {}
};